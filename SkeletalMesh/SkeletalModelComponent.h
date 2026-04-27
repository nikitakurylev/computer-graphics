#pragma once

#include "../Renderer.h"
#include "SkeletalMesh.h"
#include "../GameObject.h"
#include "../Transform.h"
#include <d3d11.h>
#include <d3dcompiler.h>

static constexpr int SHADER_MAX_BONES = 128;

class SkeletalModelComponent : public Renderer
{
public:
    SkeletalModelComponent(std::vector<SkeletalMesh>* meshes, Skeleton* skeleton);
    ~SkeletalModelComponent();

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;

    bool GetGlobalAABB(BoundingBox& outBox) override
    {
        if (!meshes_ || meshes_->empty()) return false;
        BoundingBox merged = meshes_->at(0).aabb;
        for (size_t i = 1; i < meshes_->size(); ++i)
            BoundingBox::CreateMerged(merged, merged, meshes_->at(i).aabb);
        if (gameObject)
        {
            Matrix world = gameObject->GetTransform()->GetMatrix();
            merged.Transform(outBox, world);
            return true;
        }
        return false;
    }

    // hitPosWorld  — точка попадания в мировых координатах
    // shotDir      — нормализованный вектор от камеры к цели (направление полёта шара)
    // radius       — радиус зоны деформации (world units)
    // depth        — глубина вмятины (world units)
    void ApplyDent(const Vector3& hitPosWorld,
        const Vector3& shotDir,
        float radius,
        float depth);

    Skeleton* GetSkeleton() { return skeleton_; }

private:
    std::vector<SkeletalMesh>* meshes_;
    Skeleton* skeleton_;

    std::vector<Matrix> cachedBonePalette_;
    std::vector<Matrix> cachedGlobalTransforms_;

    ID3D11VertexShader* skinnedVS_ = nullptr;
    ID3D11InputLayout* skinnedLayout_ = nullptr;
    ID3D11Buffer* boneBuffer_ = nullptr;

    HRESULT CompileShader(LPCWSTR file, LPCSTR entry, LPCSTR model, ID3DBlob** blob);
};