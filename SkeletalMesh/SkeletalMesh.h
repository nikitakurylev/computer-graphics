#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <d3d11.h>
#include <DirectXMath.h>
#include "SimpleMath.h"
#include "../SimpleTexturedDirectx11/SafeRelease.hpp"
#include "../SimpleTexturedDirectx11/Mesh.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// Максимальное количество костей влияющих на одну вершину
static constexpr int MAX_BONE_INFLUENCE = 4;
// Максимальное количество костей в скелете (для cbuffer в шейдере)
static constexpr int MAX_BONES = 128;

// ============================================================
//  Структура вершины со скелетной привязкой
// ============================================================
struct SKELETAL_VERTEX
{
    FLOAT X, Y, Z;               // Position
    FLOAT NX, NY, NZ;            // Normal
    XMFLOAT2 texcoord;           // UV
    FLOAT TX, TY, TZ;            // Tangent
    FLOAT BX, BY, BZ;            // Bitangent
    INT   BoneIndices[MAX_BONE_INFLUENCE] = { 0, 0, 0, 0 };
    FLOAT BoneWeights[MAX_BONE_INFLUENCE] = { 0, 0, 0, 0 };
};

// ============================================================
//  Одна кость скелета
// ============================================================
struct Bone
{
    std::string name;
    int         parentIndex = -1;
    Matrix      offsetMatrix;
    Matrix      localTransform;
    Matrix      bindPoseLocalTransform;
    Matrix      globalTransform;

    Matrix      preRotation = Matrix::Identity;
    bool        hasPreRotation = false;
};

// ============================================================
//  Скелет
// ============================================================
struct Skeleton
{
    std::vector<Bone>                    bones;
    std::unordered_map<std::string, int> boneNameToIndex;

    int GetBoneIndex(const std::string& name) const
    {
        auto it = boneNameToIndex.find(name);
        return (it != boneNameToIndex.end()) ? it->second : -1;
    }

    void ComputeGlobalTransforms()
    {
        for (int i = 0; i < (int)bones.size(); ++i)
        {
            if (bones[i].parentIndex < 0)
                bones[i].globalTransform = bones[i].localTransform;
            else
                bones[i].globalTransform = bones[i].localTransform * bones[bones[i].parentIndex].globalTransform;
        }
    }

    void GetFinalBoneMatrices(std::vector<Matrix>& out) const
    {
        out.resize(bones.size());
        for (int i = 0; i < (int)bones.size(); ++i)
            out[i] = bones[i].offsetMatrix * bones[i].globalTransform;
    }
};

// ============================================================
//  Скелетный меш с поддержкой CPU-деформации
// ============================================================
class SkeletalMesh
{
public:
    std::vector<SKELETAL_VERTEX> vertices;     // оригинальные вершины (bind-pose + bone weights)
    std::vector<UINT>            indices;
    Material                     material;
    BoundingBox                  aabb;

    // Накопленные смещения вершин в model-space (добавляются к X,Y,Z при Upload)
    std::vector<Vector3>         displacements;
    bool                         isDirty = false; // есть несгруженные изменения

    SkeletalMesh() = default;
    ~SkeletalMesh() { Close(); }

    // --------------------------------------------------------
    //  SetupGPU — создаёт DYNAMIC VBO для поддержки деформации
    // --------------------------------------------------------
    void SetupGPU(ID3D11Device* dev)
    {
        dev_ = dev;

        // Вершины — DYNAMIC + CPU_WRITE, чтобы обновлять смещения
        {
            D3D11_BUFFER_DESC vbd = {};
            vbd.Usage = D3D11_USAGE_DYNAMIC;
            vbd.ByteWidth = (UINT)(sizeof(SKELETAL_VERTEX) * vertices.size());
            vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = vertices.data();
            dev->CreateBuffer(&vbd, &initData, &vertexBuffer_);
        }

        // Индексы — IMMUTABLE
        {
            D3D11_BUFFER_DESC ibd = {};
            ibd.Usage = D3D11_USAGE_IMMUTABLE;
            ibd.ByteWidth = (UINT)(sizeof(UINT) * indices.size());
            ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = indices.data();
            dev->CreateBuffer(&ibd, &initData, &indexBuffer_);
        }

        // Material CB
        {
            D3D11_BUFFER_DESC mbd = {};
            mbd.Usage = D3D11_USAGE_DEFAULT;
            mbd.ByteWidth = sizeof(MaterialConstants);
            mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            dev->CreateBuffer(&mbd, nullptr, &materialBuffer_);
        }

        // AABB по bind-pose
        if (!vertices.empty())
            BoundingBox::CreateFromPoints(aabb, vertices.size(),
                reinterpret_cast<const XMFLOAT3*>(&vertices[0].X), sizeof(SKELETAL_VERTEX));

        // Инициализируем массив смещений нулями
        displacements.assign(vertices.size(), Vector3::Zero);
    }

    // --------------------------------------------------------
    //  ApplyDent — добавляет вмятину в точке hitPosModel (model space)
    //
    //  Алгоритм: для каждой вершины вычисляем её скиннированную
    //  позицию в model-space через упрощённый CPU-skinning,
    //  если расстояние до hitPosModel < radius — смещаем вершину
    //  внутрь (по инвертированной нормали) с плавным спадом.
    // --------------------------------------------------------
    void ApplyDent(const Vector3& hitPosModel,
        float radius,
        float depth,
        const std::vector<Matrix>& bonePalette)
    {
        const float r2 = radius * radius;

        for (size_t i = 0; i < vertices.size(); ++i)
        {
            const SKELETAL_VERTEX& v = vertices[i];

            // CPU skinning — вычисляем текущую позицию вершины
            Vector4 skinnedPos(0, 0, 0, 0);
            for (int b = 0; b < MAX_BONE_INFLUENCE; ++b)
            {
                float w = v.BoneWeights[b];
                if (w < 1e-5f) continue;
                int   idx = v.BoneIndices[b];
                if (idx < 0 || idx >= (int)bonePalette.size()) continue;

                Vector4 localPos(v.X + displacements[i].x,
                    v.Y + displacements[i].y,
                    v.Z + displacements[i].z, 1.0f);
                skinnedPos += Vector4::Transform(localPos, bonePalette[idx]) * w;
            }

            Vector3 pos3(skinnedPos.x, skinnedPos.y, skinnedPos.z);
            float dist2 = Vector3::DistanceSquared(pos3, hitPosModel);
            if (dist2 >= r2) continue;

            // Плавный спад по расстоянию (smoothstep-like)
            float t = 1.0f - sqrtf(dist2) / radius;
            float magnitude = depth * t * t;

            // Направление смещения — от точки удара к вершине (но внутрь)
            Vector3 dir = pos3 - hitPosModel;
            if (dir.LengthSquared() < 1e-8f)
                dir = Vector3(-v.NX, -v.NY, -v.NZ); // если прямо в точке — по нормали внутрь
            else
                dir.Normalize();

            // Смещаем в model-space (инвертируем skinning примерно через нормаль)
            // Проще: смещаем bind-pose позицию по нормали вершины внутрь
            Vector3 dentDir(-v.NX, -v.NY, -v.NZ);
            if (dentDir.LengthSquared() < 1e-8f) dentDir = -dir;

            displacements[i] += dentDir * magnitude;
        }

        isDirty = true;
    }

    // --------------------------------------------------------
    //  UploadDeformed — если есть изменения, загружает
    //  деформированные вершины в DYNAMIC VBO
    // --------------------------------------------------------
    void UploadDeformed(ID3D11DeviceContext* ctx)
    {
        if (!isDirty || !vertexBuffer_) return;

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (FAILED(ctx->Map(vertexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            return;

        SKELETAL_VERTEX* dst = static_cast<SKELETAL_VERTEX*>(mapped.pData);
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            dst[i] = vertices[i];
            dst[i].X += displacements[i].x;
            dst[i].Y += displacements[i].y;
            dst[i].Z += displacements[i].z;
        }

        ctx->Unmap(vertexBuffer_, 0);
        isDirty = false;
    }

    void Draw(ID3D11DeviceContext* ctx)
    {
        if (!vertexBuffer_ || !indexBuffer_) return;

        UINT stride = sizeof(SKELETAL_VERTEX);
        UINT offset = 0;
        ctx->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
        ctx->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);

        if (materialBuffer_)
        {
            MaterialConstants mc = { material.baseColorFactor, material.materialParams };
            ctx->UpdateSubresource(materialBuffer_, 0, nullptr, &mc, 0, 0);
            ctx->PSSetConstantBuffers(2, 1, &materialBuffer_);
        }

        ID3D11ShaderResourceView* srvs[5] = {
            material.albedo ? material.albedo : nullptr,
            material.ao ? material.ao : nullptr,
            material.metallic ? material.metallic : nullptr,
            material.roughness ? material.roughness : nullptr,
            material.normal ? material.normal : nullptr
        };
        ctx->PSSetShaderResources(0, 5, srvs);
        ctx->DrawIndexed((UINT)indices.size(), 0, 0);
    }

    void Close()
    {
        SafeRelease(vertexBuffer_);
        SafeRelease(indexBuffer_);
        SafeRelease(materialBuffer_);
    }

private:
    ID3D11Device* dev_ = nullptr;
    ID3D11Buffer* vertexBuffer_ = nullptr;
    ID3D11Buffer* indexBuffer_ = nullptr;
    ID3D11Buffer* materialBuffer_ = nullptr;
};