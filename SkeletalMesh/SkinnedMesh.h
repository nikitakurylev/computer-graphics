#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "SimpleMath.h"
#include "Skeleton.h"
#include "../SimpleTexturedDirectx11/SafeRelease.hpp"
#include <stdexcept>

using namespace DirectX;
using namespace DirectX::SimpleMath;

// ============================================================================
// SkinnedMesh.h
// Mesh with bone weights for skeletal animation
// ============================================================================

namespace Animation {

    // ------------------------------------------------------------------------
    // SkinnedVertex - vertex with bone influences
    // 
    // Each vertex can be influenced by up to 4 bones
    // ------------------------------------------------------------------------
    struct SkinnedVertex {
        // Standard attributes
        float x, y, z;                  // Position
        float nx, ny, nz;               // Normal
        float u, v;                     // Texture coordinates
        float tx, ty, tz;               // Tangent
        float bx, by, bz;               // Bitangent
        
        // Skinning data
        int boneIds[4];                 // Bone indices (up to 4)
        float weights[4];               // Bone weights (must sum to 1.0)
        
        SkinnedVertex() {
            x = y = z = 0;
            nx = ny = nz = 0;
            u = v = 0;
            tx = ty = tz = 0;
            bx = by = bz = 0;
            
            for (int i = 0; i < 4; i++) {
                boneIds[i] = 0;
                weights[i] = 0.0f;
            }
        }
        
        // Get position as Vector3
        Vector3 GetPosition() const {
            return Vector3(x, y, z);
        }
        
        // Set position from Vector3
        void SetPosition(const Vector3& pos) {
            x = pos.x;
            y = pos.y;
            z = pos.z;
        }
        
        // Get normal as Vector3
        Vector3 GetNormal() const {
            return Vector3(nx, ny, nz);
        }
        
        // Set normal from Vector3
        void SetNormal(const Vector3& normal) {
            nx = normal.x;
            ny = normal.y;
            nz = normal.z;
        }
    };
    
    // ------------------------------------------------------------------------
    // Material structure (same as regular mesh)
    // ------------------------------------------------------------------------
    struct SkinnedMaterial {
        ID3D11ShaderResourceView* albedo;
        ID3D11ShaderResourceView* ao;
        ID3D11ShaderResourceView* metallic;
        ID3D11ShaderResourceView* roughness;
        ID3D11ShaderResourceView* normal;
        XMFLOAT4 baseColorFactor;
        XMFLOAT4 materialParams;
        
        SkinnedMaterial() {
            albedo = nullptr;
            ao = nullptr;
            metallic = nullptr;
            roughness = nullptr;
            normal = nullptr;
            baseColorFactor = XMFLOAT4(1, 1, 1, 1);
            materialParams = XMFLOAT4(0, 1, 1, 0);
        }
    };
    
    // ------------------------------------------------------------------------
    // SkinnedMesh - mesh with skeletal animation support
    // 
    // Features:
    // - Dynamic vertex buffer (for deformation)
    // - Bone weight data
    // - AABB calculation
    // - Material support
    // ------------------------------------------------------------------------
    class SkinnedMesh {
    public:
        SkinnedMesh(ID3D11Device* device, 
                   const std::vector<SkinnedVertex>& verts,
                   const std::vector<UINT>& inds,
                   const SkinnedMaterial& mat,
                   Skeleton* skel = nullptr)
            : device_(device)
            , vertices_(verts)
            , indices_(inds)
            , material_(mat)
            , skeleton_(skel)
            , vertexBuffer_(nullptr)
            , indexBuffer_(nullptr)
            , materialBuffer_(nullptr)
            , isDirty_(false) {
            
            // Calculate AABB
            UpdateAABB();
            
            // Create GPU buffers
            CreateBuffers();
            CreateMaterialBuffer();
        }
        
        ~SkinnedMesh() {
            Release();
        }
        
        // --------------------------------------------------------------------
        // Draw the mesh
        // --------------------------------------------------------------------
        void Draw(ID3D11DeviceContext* context) {
            // Update vertex buffer if mesh was deformed
            if (isDirty_) {
                UpdateVertexBuffer(context);
                isDirty_ = false;
            }
            
            UINT stride = sizeof(SkinnedVertex);
            UINT offset = 0;
            
            context->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
            context->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);
            
            // Set material
            if (materialBuffer_) {
                struct MaterialConstants {
                    XMFLOAT4 baseColorFactor;
                    XMFLOAT4 materialParams;
                } constants = { material_.baseColorFactor, material_.materialParams };
                
                context->UpdateSubresource(materialBuffer_, 0, nullptr, &constants, 0, 0);
                context->PSSetConstantBuffers(2, 1, &materialBuffer_);
            }
            
            ID3D11ShaderResourceView* srvs[] = { 
                material_.albedo, 
                material_.ao, 
                material_.metallic, 
                material_.roughness, 
                material_.normal 
            };
            context->PSSetShaderResources(0, 5, srvs);
            
            context->DrawIndexed(static_cast<UINT>(indices_.size()), 0, 0);
        }
        
        // --------------------------------------------------------------------
        // Access vertices (for deformation)
        // --------------------------------------------------------------------
        std::vector<SkinnedVertex>& GetVertices() {
            isDirty_ = true;
            return vertices_;
        }
        
        const std::vector<SkinnedVertex>& GetVertices() const {
            return vertices_;
        }
        
        // --------------------------------------------------------------------
        // Access indices
        // --------------------------------------------------------------------
        const std::vector<UINT>& GetIndices() const {
            return indices_;
        }
        
        // --------------------------------------------------------------------
        // Get AABB
        // --------------------------------------------------------------------
        const BoundingBox& GetAABB() const {
            return aabb_;
        }
        
        // --------------------------------------------------------------------
        // Update AABB from current vertices
        // --------------------------------------------------------------------
        void UpdateAABB() {
            if (!vertices_.empty()) {
                // DirectX expects XMFLOAT3*, but our vertices have different layout
                std::vector<XMFLOAT3> positions;
                positions.reserve(vertices_.size());
                
                for (const auto& v : vertices_) {
                    positions.push_back(XMFLOAT3(v.x, v.y, v.z));
                }
                
                BoundingBox::CreateFromPoints(aabb_, positions.size(), 
                                             &positions[0], sizeof(XMFLOAT3));
            }
        }
        
        // --------------------------------------------------------------------
        // Mark mesh as dirty (needs GPU update)
        // --------------------------------------------------------------------
        void MarkDirty() {
            isDirty_ = true;
        }
        
        // --------------------------------------------------------------------
        // Get skeleton
        // --------------------------------------------------------------------
        Skeleton* GetSkeleton() const {
            return skeleton_;
        }
        
        // --------------------------------------------------------------------
        // Set skeleton
        // --------------------------------------------------------------------
        void SetSkeleton(Skeleton* skel) {
            skeleton_ = skel;
        }
        
    private:
        ID3D11Device* device_;
        std::vector<SkinnedVertex> vertices_;
        std::vector<UINT> indices_;
        SkinnedMaterial material_;
        Skeleton* skeleton_;
        BoundingBox aabb_;
        bool isDirty_;
        
        // GPU resources
        ID3D11Buffer* vertexBuffer_;
        ID3D11Buffer* indexBuffer_;
        ID3D11Buffer* materialBuffer_;
        
        // --------------------------------------------------------------------
        // Create vertex and index buffers
        // --------------------------------------------------------------------
        void CreateBuffers() {
            HRESULT hr;
            
            // Vertex buffer (DYNAMIC for deformation support)
            D3D11_BUFFER_DESC vbd = {};
            vbd.Usage = D3D11_USAGE_DYNAMIC;
            vbd.ByteWidth = static_cast<UINT>(sizeof(SkinnedVertex) * vertices_.size());
            vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;  // Allow CPU updates
            vbd.MiscFlags = 0;
            
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = vertices_.data();
            
            hr = device_->CreateBuffer(&vbd, &initData, &vertexBuffer_);
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create skinned vertex buffer.");
            }
            
            // Index buffer
            D3D11_BUFFER_DESC ibd = {};
            ibd.Usage = D3D11_USAGE_IMMUTABLE;
            ibd.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices_.size());
            ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            ibd.CPUAccessFlags = 0;
            ibd.MiscFlags = 0;
            
            initData.pSysMem = indices_.data();
            
            hr = device_->CreateBuffer(&ibd, &initData, &indexBuffer_);
            if (FAILED(hr)) {
                Release();
                throw std::runtime_error("Failed to create skinned index buffer.");
            }
        }
        
        // --------------------------------------------------------------------
        // Create material constant buffer
        // --------------------------------------------------------------------
        void CreateMaterialBuffer() {
            D3D11_BUFFER_DESC mbd = {};
            mbd.Usage = D3D11_USAGE_DEFAULT;
            mbd.ByteWidth = sizeof(XMFLOAT4) * 2;  // baseColorFactor + materialParams
            mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            mbd.CPUAccessFlags = 0;
            
            HRESULT hr = device_->CreateBuffer(&mbd, nullptr, &materialBuffer_);
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create material buffer.");
            }
        }
        
        // --------------------------------------------------------------------
        // Update vertex buffer on GPU (for deformation)
        // --------------------------------------------------------------------
        void UpdateVertexBuffer(ID3D11DeviceContext* context) {
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = context->Map(vertexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            
            if (SUCCEEDED(hr)) {
                memcpy(mappedResource.pData, vertices_.data(), 
                       sizeof(SkinnedVertex) * vertices_.size());
                context->Unmap(vertexBuffer_, 0);
            }
        }
        
        // --------------------------------------------------------------------
        // Release GPU resources
        // --------------------------------------------------------------------
        void Release() {
            SafeRelease(vertexBuffer_);
            SafeRelease(indexBuffer_);
            SafeRelease(materialBuffer_);
        }
    };

} // namespace Animation
