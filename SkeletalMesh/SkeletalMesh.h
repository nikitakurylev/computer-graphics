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
    FLOAT BoneWeights[4] = { 0, 0, 0, 0 };
};

// ============================================================
//  Одна кость скелета
// ============================================================
struct Bone
{
    std::string name;
    int         parentIndex = -1;        // -1 = корневая кость
    Matrix      offsetMatrix;            // bind-pose inverse (bone -> mesh space)
    Matrix      localTransform;          // локальный трансформ в T-pose (из узла сцены)
    Matrix      globalTransform;         // мировой трансформ в T-pose (кэш)
};

// ============================================================
//  Скелет — набор костей с иерархией
// ============================================================
struct Skeleton
{
    std::vector<Bone>                  bones;
    std::unordered_map<std::string, int> boneNameToIndex;

    // Возвращает индекс кости по имени, -1 если не найдена
    int GetBoneIndex(const std::string& name) const
    {
        auto it = boneNameToIndex.find(name);
        return (it != boneNameToIndex.end()) ? it->second : -1;
    }

    // Вычисляет глобальные трансформы всех костей (T-pose / текущее состояние)
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

    // Заполняет массив финальных матриц (offsetMatrix * globalTransform) для шейдера
    void GetFinalBoneMatrices(std::vector<Matrix>& out) const
    {
        out.resize(bones.size());
        for (int i = 0; i < (int)bones.size(); ++i)
            out[i] = bones[i].offsetMatrix * bones[i].globalTransform;
    }
};

// ============================================================
//  Скелетный меш — аналог Mesh, но со skinned вершинами и костями
// ============================================================
class SkeletalMesh
{
public:
    std::vector<SKELETAL_VERTEX> vertices;
    std::vector<UINT>            indices;
    Material                     material;
    BoundingBox                  aabb;

    SkeletalMesh() = default;
    ~SkeletalMesh() { Close(); }

    // Создаёт GPU-буферы. Вызывается один раз после загрузки.
    void SetupGPU(ID3D11Device* dev)
    {
        dev_ = dev;

        // Vertex buffer — DYNAMIC, т.к. в будущем можно делать CPU skinning
        // Но мы используем GPU skinning через constant buffer, поэтому IMMUTABLE
        {
            D3D11_BUFFER_DESC vbd = {};
            vbd.Usage = D3D11_USAGE_IMMUTABLE;
            vbd.ByteWidth = (UINT)(sizeof(SKELETAL_VERTEX) * vertices.size());
            vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = vertices.data();
            dev->CreateBuffer(&vbd, &initData, &vertexBuffer_);
        }

        // Index buffer
        {
            D3D11_BUFFER_DESC ibd = {};
            ibd.Usage = D3D11_USAGE_IMMUTABLE;
            ibd.ByteWidth = (UINT)(sizeof(UINT) * indices.size());
            ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = indices.data();
            dev->CreateBuffer(&ibd, &initData, &indexBuffer_);
        }

        // Material constant buffer
        {
            D3D11_BUFFER_DESC mbd = {};
            mbd.Usage = D3D11_USAGE_DEFAULT;
            mbd.ByteWidth = sizeof(MaterialConstants);
            mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            dev->CreateBuffer(&mbd, nullptr, &materialBuffer_);
        }

        // AABB
        if (!vertices.empty())
            BoundingBox::CreateFromPoints(aabb, vertices.size(),
                reinterpret_cast<const XMFLOAT3*>(&vertices[0].X), sizeof(SKELETAL_VERTEX));
    }

    void Draw(ID3D11DeviceContext* ctx)
    {
        if (!vertexBuffer_ || !indexBuffer_) return;

        UINT stride = sizeof(SKELETAL_VERTEX);
        UINT offset = 0;
        ctx->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
        ctx->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);

        // Material constants
        if (materialBuffer_)
        {
            MaterialConstants mc = { material.baseColorFactor, material.materialParams };
            ctx->UpdateSubresource(materialBuffer_, 0, nullptr, &mc, 0, 0);
            ctx->PSSetConstantBuffers(2, 1, &materialBuffer_);
        }

        // Текстуры — только если все слоты валидны
        // (nullptr SRV корректен для DX11 и просто отключает слот)
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