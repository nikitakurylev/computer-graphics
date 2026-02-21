#pragma once

#include "../Renderer.h"
#include "SkeletalMesh.h"
#include "../GameObject.h"
#include "../Transform.h"
#include <d3d11.h>
#include <d3dcompiler.h>

// Максимальное количество костей — должно совпадать с MAX_BONES в шейдере
// и с константой в SkeletalMesh.h
static constexpr int SHADER_MAX_BONES = 128;

// ============================================================
//  SkeletalModelComponent
//
//  Компонент, который:
//  - хранит ссылку на загруженные SkeletalMesh'и и Skeleton
//  - в Initialize() создаёт собственный skinned vertex shader + input layout
//  - в Draw() обновляет bone palette в CB3, рисует все submesh'и
// ============================================================
class SkeletalModelComponent : public Renderer
{
public:
    // Принимает данные, уже загруженные SkeletalModelLoader'ом.
    // Компонент не владеет meshes/skeleton (пусть хранятся снаружи),
    // но для простоты копируем их сюда.
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

    // Публичный доступ к скелету, чтобы AnimationComponent мог его изменять
    Skeleton* GetSkeleton() { return skeleton_; }

private:
    std::vector<SkeletalMesh>* meshes_;
    Skeleton* skeleton_;

    ID3D11VertexShader* skinnedVS_ = nullptr;
    ID3D11InputLayout* skinnedLayout_ = nullptr;
    ID3D11Buffer* boneBuffer_ = nullptr; // CB3 — bone palette

    HRESULT CompileShader(LPCWSTR file, LPCSTR entry, LPCSTR model, ID3DBlob** blob);
};