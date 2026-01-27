#ifndef MESH_H
#define MESH_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>

using namespace DirectX;

#include "SafeRelease.hpp"

struct VERTEX {
	FLOAT X, Y, Z;
	FLOAT NX, NY, NZ;
	XMFLOAT2 texcoord;
	FLOAT TX, TY, TZ;
	FLOAT BX, BY, BZ;
};

struct Texture {
	std::string type;
	std::string path;
	ID3D11ShaderResourceView *texture;

	void Release() {
		SafeRelease(texture);
	}
};

struct Material {
	ID3D11ShaderResourceView* albedo;
	ID3D11ShaderResourceView* ao;
	ID3D11ShaderResourceView* metallic;
	ID3D11ShaderResourceView* roughness;
	ID3D11ShaderResourceView* normal;
	XMFLOAT4 baseColorFactor;
	XMFLOAT4 materialParams; // x=metallic, y=roughness, z=ao, w=unused
};

struct MaterialConstants {
	XMFLOAT4 baseColorFactor;
	XMFLOAT4 materialParams;
};

class Mesh {
public:
    std::vector<VERTEX> vertices_;
    std::vector<UINT> indices_;
    std::vector<Texture> textures_;
	Material material_;
    ID3D11Device *dev_;
    BoundingBox aabb;

    Mesh(ID3D11Device *dev, const std::vector<VERTEX>& vertices, const std::vector<UINT>& indices, const std::vector<Texture>& textures, const Material& material) :
            vertices_(vertices),
            indices_(indices),
            textures_(textures),
			material_(material),
            dev_(dev),
            VertexBuffer_(nullptr),
            IndexBuffer_(nullptr),
			materialBuffer_(nullptr) {
        if (!vertices.empty()) {
            BoundingBox::CreateFromPoints(aabb, vertices.size(), reinterpret_cast<const XMFLOAT3*>(&vertices[0].X), sizeof(VERTEX));
        }
        this->setupMesh(this->dev_);
		this->setupMaterialBuffer(this->dev_);
    }

    void Draw(ID3D11DeviceContext *devcon) {
        UINT stride = sizeof(VERTEX);
        UINT offset = 0;

        devcon->IASetVertexBuffers(0, 1, &VertexBuffer_, &stride, &offset);
        devcon->IASetIndexBuffer(IndexBuffer_, DXGI_FORMAT_R32_UINT, 0);

		if (materialBuffer_ != nullptr) {
			MaterialConstants constants = { material_.baseColorFactor, material_.materialParams };
			devcon->UpdateSubresource(materialBuffer_, 0, nullptr, &constants, 0, 0);
			devcon->PSSetConstantBuffers(2, 1, &materialBuffer_);
		}

		ID3D11ShaderResourceView* srvs[] = { material_.albedo, material_.ao, material_.metallic, material_.roughness, material_.normal };
        devcon->PSSetShaderResources(0, 5, srvs);

        devcon->DrawIndexed(static_cast<UINT>(indices_.size()), 0, 0);
    }

    void Close() {
        SafeRelease(VertexBuffer_);
        SafeRelease(IndexBuffer_);
		SafeRelease(materialBuffer_);
    }
private:
    // Render data
    ID3D11Buffer *VertexBuffer_, *IndexBuffer_, *materialBuffer_;

    // Functions
    // Initializes all the buffer objects/arrays
    void setupMesh(ID3D11Device *dev) {
        HRESULT hr;

        D3D11_BUFFER_DESC vbd;
        vbd.Usage = D3D11_USAGE_IMMUTABLE;
        vbd.ByteWidth = static_cast<UINT>(sizeof(VERTEX) * vertices_.size());
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbd.CPUAccessFlags = 0;
        vbd.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &vertices_[0];

        hr = dev->CreateBuffer(&vbd, &initData, &VertexBuffer_);
        if (FAILED(hr)) {
            Close();
            throw std::runtime_error("Failed to create vertex buffer.");
        }

        D3D11_BUFFER_DESC ibd;
        ibd.Usage = D3D11_USAGE_IMMUTABLE;
        ibd.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices_.size());
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibd.CPUAccessFlags = 0;
        ibd.MiscFlags = 0;

        initData.pSysMem = &indices_[0];

        hr = dev->CreateBuffer(&ibd, &initData, &IndexBuffer_);
        if (FAILED(hr)) {
            Close();
            throw std::runtime_error("Failed to create index buffer.");
        }
    }

	void setupMaterialBuffer(ID3D11Device* dev) {
		D3D11_BUFFER_DESC mbd = {};
		mbd.Usage = D3D11_USAGE_DEFAULT;
		mbd.ByteWidth = sizeof(MaterialConstants);
		mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		mbd.CPUAccessFlags = 0;
		mbd.MiscFlags = 0;
		mbd.StructureByteStride = 0;

		auto hr = dev->CreateBuffer(&mbd, nullptr, &materialBuffer_);
		if (FAILED(hr)) {
			Close();
			throw std::runtime_error("Failed to create material buffer.");
		}
	}
};

#endif
