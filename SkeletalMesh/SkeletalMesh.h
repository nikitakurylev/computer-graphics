#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <d3d11.h>
#include <DirectXMath.h>
#include "SimpleMath.h"
#include "../SimpleTexturedDirectx11/SafeRelease.hpp"
#include "../SimpleTexturedDirectx11/Mesh.h"
#include "../BenchmarkLogger.h"
#include <chrono>

using namespace DirectX;
using namespace DirectX::SimpleMath;

static constexpr int MAX_BONE_INFLUENCE = 4;
static constexpr int MAX_BONES = 128;

struct SKELETAL_VERTEX
{
    FLOAT X, Y, Z;
    FLOAT NX, NY, NZ;
    XMFLOAT2 texcoord;
    FLOAT TX, TY, TZ;
    FLOAT BX, BY, BZ;
    INT   BoneIndices[MAX_BONE_INFLUENCE] = { 0, 0, 0, 0 };
    FLOAT BoneWeights[MAX_BONE_INFLUENCE] = { 0, 0, 0, 0 };
};

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
                bones[i].globalTransform = bones[i].localTransform
                * bones[bones[i].parentIndex].globalTransform;
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
class SkeletalMesh
{
public:
    // vertices[] — деформированные координаты (меняются при каждом ударе)
    // originalVertices[] — неизменяемые bind-pose координаты загрузки
    //   используются ТОЛЬКО для CPU-skinning и поиска вершин в радиусе
    std::vector<SKELETAL_VERTEX> vertices;
    std::vector<SKELETAL_VERTEX> originalVertices; // копия из SetupGPU, не меняется
    std::vector<UINT>            indices;
    Material                     material;
    BoundingBox                  aabb;
    bool                         dirty = false;

    SkeletalMesh() = default;
    ~SkeletalMesh() { Close(); }

    void SetupGPU(ID3D11Device* dev)
    {
        dev_ = dev;

        // Сохраняем оригинальные вершины — они никогда не изменятся
        originalVertices = vertices;

        {
            D3D11_BUFFER_DESC vbd = {};
            vbd.Usage = D3D11_USAGE_DYNAMIC;
            vbd.ByteWidth = (UINT)(sizeof(SKELETAL_VERTEX) * vertices.size());
            vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            D3D11_SUBRESOURCE_DATA init = {};
            init.pSysMem = vertices.data();
            dev->CreateBuffer(&vbd, &init, &vertexBuffer_);
        }
        {
            D3D11_BUFFER_DESC ibd = {};
            ibd.Usage = D3D11_USAGE_IMMUTABLE;
            ibd.ByteWidth = (UINT)(sizeof(UINT) * indices.size());
            ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA init = {};
            init.pSysMem = indices.data();
            dev->CreateBuffer(&ibd, &init, &indexBuffer_);
        }
        {
            D3D11_BUFFER_DESC mbd = {};
            mbd.Usage = D3D11_USAGE_DEFAULT;
            mbd.ByteWidth = sizeof(MaterialConstants);
            mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            dev->CreateBuffer(&mbd, nullptr, &materialBuffer_);
        }

        if (!vertices.empty())
            BoundingBox::CreateFromPoints(aabb, vertices.size(),
                reinterpret_cast<const XMFLOAT3*>(&vertices[0].X),
                sizeof(SKELETAL_VERTEX));
    }

    // --------------------------------------------------------
    //  ApplyDent
    //
    //  Все входные параметры — в model space (World убран снаружи).
    //
    //  Ключевые исправления относительно предыдущей версии:
    //
    //  1. CPU-skinning считается по originalVertices (оригинальный bind-pose),
    //     а НЕ по vertices (которые уже деформированы). Это гарантирует что
    //     радиус удара всегда проверяется по реальной анатомической позиции
    //     вершины, независимо от накопленных деформаций. Без этого после
    //     нескольких ударов вершины уходят из-под точки попадания и
    //     деформация прекращается.
    //
    //  2. Смещение применяется как +shotDir (а не -shotDir). shotDir
    //     указывает ОТ камеры К цели — то есть это и есть направление
    //     «внутрь» от точки зрения стрелка. Вмятина продавливается
    //     в ту сторону, куда летел шар.
    //
    //  3. Смещение в model space переводится в bind-pose space через
    //     inverse(globalTransform) доминирующей кости. Это нужно потому что
    //     vertices[] хранят координаты ДО применения globalTransform,
    //     а после удара шейдер снова применит globalTransform —
    //     без этого шага деформация была бы двойной или перевёрнутой.
    // --------------------------------------------------------
    void ApplyDent(const Vector3& hitPosModel,
        const Vector3& shotDirModel,
        float          radius,
        float          depth,
        const std::vector<Matrix>& bonePalette,
        const std::vector<Matrix>& globalTransforms)
    {
        BENCH_SCOPE("ApplyDent_ms");
        const float r2 = radius * radius;

        for (size_t i = 0; i < originalVertices.size(); ++i)
        {
            const SKELETAL_VERTEX& orig = originalVertices[i]; // только для поиска
            SKELETAL_VERTEX& vert = vertices[i];         // сюда пишем

            // 1. CPU-skinning по ОРИГИНАЛЬНЫМ вершинам → текущая model-pos
            Vector4 modelPos4(0, 0, 0, 0);
            for (int b = 0; b < MAX_BONE_INFLUENCE; ++b)
            {
                float w = orig.BoneWeights[b];
                if (w < 1e-6f) continue;
                int idx = orig.BoneIndices[b];
                if (idx < 0 || idx >= (int)bonePalette.size()) continue;

                Vector4 lp(orig.X, orig.Y, orig.Z, 1.0f);
                modelPos4 += Vector4::Transform(lp, bonePalette[idx]) * w;
            }
            Vector3 modelPos(modelPos4.x, modelPos4.y, modelPos4.z);

            // 2. Проверяем попадание в радиус
            float dist2 = Vector3::DistanceSquared(modelPos, hitPosModel);
            if (dist2 >= r2) continue;

            float dist = sqrtf(dist2);

            // Квадратичный спад: 1 в центре, 0 на краю
            float t = 1.0f - (dist / radius);
            float push = depth * t * t;

            // 3. Смещение в model space: +shotDir = вдоль направления полёта шара
            Vector3 deltaModel = shotDirModel * push;

            // 4. Доминирующая кость
            int   domBone = 0;
            float domWeight = -1.0f;
            for (int b = 0; b < MAX_BONE_INFLUENCE; ++b)
            {
                if (orig.BoneWeights[b] > domWeight)
                {
                    domWeight = orig.BoneWeights[b];
                    domBone = orig.BoneIndices[b];
                }
            }
            if (domBone < 0 || domBone >= (int)globalTransforms.size()) continue;

            // 5. Переводим delta из model space в bind-pose space кости
            //    vertices хранятся в пространстве ДО globalTransform,
            //    поэтому отменяем globalTransform доминирующей кости
            Matrix invGlobal;
            globalTransforms[domBone].Invert(invGlobal);
            Vector3 deltaLocal = Vector3::TransformNormal(deltaModel, invGlobal);

            // 6. Запекаем в деформированные вершины
            vert.X += deltaLocal.x;
            vert.Y += deltaLocal.y;
            vert.Z += deltaLocal.z;
        }

        dirty = true;
        // Считаем максимальное смещение вершины (глубина кратера)
        float maxDelta2 = 0.0f;
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            float dx = vertices[i].X - originalVertices[i].X;
            float dy = vertices[i].Y - originalVertices[i].Y;
            float dz = vertices[i].Z - originalVertices[i].Z;
            float d2 = dx * dx + dy * dy + dz * dz;
            if (d2 > maxDelta2) maxDelta2 = d2;
        }
        BenchmarkLogger::Instance().LogValue("CraterDepth_modelUnits",
            sqrtf(maxDelta2));
    }

    void UploadToGPU(ID3D11DeviceContext* ctx)
    {
        if (!dirty || !vertexBuffer_) return;

        auto t0 = std::chrono::high_resolution_clock::now();
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (FAILED(ctx->Map(vertexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            return;
        memcpy(mapped.pData, vertices.data(), sizeof(SKELETAL_VERTEX) * vertices.size());
        ctx->Unmap(vertexBuffer_, 0);
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        BenchmarkLogger::Instance().LogValue("UploadToGPU_ms", ms);
        dirty = false;
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