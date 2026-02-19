#pragma once
#include "../Renderer.h"
#include "SkinnedMesh.h"
#include "Skeleton.h"

// ============================================================================
// SkinnedModelComponent.h
// Component for rendering skinned meshes with skeletal animation
// ============================================================================

class SkinnedModelComponent : public Renderer {
public:
    SkinnedModelComponent(Animation::SkinnedMesh* mesh, Animation::Skeleton* skeleton)
        : mesh_(mesh)
        , skeleton_(skeleton)
        , boneMatrixBuffer_(nullptr) {
    }
    
    virtual ~SkinnedModelComponent() {
        if (boneMatrixBuffer_) {
            boneMatrixBuffer_->Release();
            boneMatrixBuffer_ = nullptr;
        }
    }
    
    // ------------------------------------------------------------------------
    // Initialize - create bone matrix constant buffer
    // ------------------------------------------------------------------------
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override {
        if (!skeleton_) return;
        
        // Create constant buffer for bone matrices (register b1)
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX) * 128;  // Max 128 bones
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        
        HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &boneMatrixBuffer_);
        if (FAILED(hr)) {
            printf("[SkinnedModelComponent] Failed to create bone matrix buffer\n");
        }
    }
    
    // ------------------------------------------------------------------------
    // Draw - render the skinned mesh
    // ------------------------------------------------------------------------
    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override {
        if (!mesh_ || !skeleton_) return;
        
        // Update bone matrices in constant buffer
        UpdateBoneMatrices(context);
        
        // Bind bone matrix buffer to shader (register b1)
        context->VSSetConstantBuffers(1, 1, &boneMatrixBuffer_);
        
        // Draw the mesh
        mesh_->Draw(context);
    }
    
    // ------------------------------------------------------------------------
    // Get AABB for frustum culling
    // ------------------------------------------------------------------------
    bool GetGlobalAABB(BoundingBox& outBox) override {
        if (!mesh_) return false;
        
        BoundingBox localAABB = mesh_->GetAABB();
        
        if (gameObject) {
            Matrix world = gameObject->GetTransform()->GetMatrix();
            localAABB.Transform(outBox, world);
            return true;
        }
        
        outBox = localAABB;
        return true;
    }
    
    // ------------------------------------------------------------------------
    // Get skeleton (for animation component)
    // ------------------------------------------------------------------------
    Animation::Skeleton* GetSkeleton() const {
        return skeleton_;
    }
    
    // ------------------------------------------------------------------------
    // Get mesh (for deformation)
    // ------------------------------------------------------------------------
    Animation::SkinnedMesh* GetMesh() const {
        return mesh_;
    }
    
protected:
    Animation::SkinnedMesh* mesh_;
    Animation::Skeleton* skeleton_;
    ID3D11Buffer* boneMatrixBuffer_;
    
    // ------------------------------------------------------------------------
    // Update bone matrices in GPU constant buffer
    // ------------------------------------------------------------------------
    void UpdateBoneMatrices(ID3D11DeviceContext* context) {
        if (!boneMatrixBuffer_ || !skeleton_) return;
        
        // Get bone matrices from skeleton
        const auto& boneMatrices = skeleton_->GetBoneMatrices();
        
        if (boneMatrices.empty()) return;
        
        // Map constant buffer
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = context->Map(boneMatrixBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        
        if (SUCCEEDED(hr)) {
            // Copy bone matrices
            size_t size = std::min(boneMatrices.size(), (size_t)128) * sizeof(DirectX::XMMATRIX);
            memcpy(mappedResource.pData, boneMatrices.data(), size);
            
            context->Unmap(boneMatrixBuffer_, 0);
        }
    }
};
