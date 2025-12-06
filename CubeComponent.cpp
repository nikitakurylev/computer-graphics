#include "CubeComponent.h"
#include <directxmath.h>
#include <iostream>
#include "Game.h"

CubeComponent::CubeComponent()
{
	SetSize(0.5f, 0.5f, 0.5f);
}

void CubeComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	points[0].texCoord = DirectX::XMFLOAT2(1, 1);
	points[1].texCoord = DirectX::XMFLOAT2(-1, -1);
	points[2].texCoord = DirectX::XMFLOAT2(-1, 1);
	points[3].texCoord = DirectX::XMFLOAT2(1, -1);


	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(Vertex) * std::size(points);

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = points;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

	int indeces[] = {
		0,1,
		1,2,
		2,3,
		4,5,
		6,7,
		0,3,
		0,4,
		1,5,
		2,6,
		3,7,
		4,7,
		5,6
	};
	auto indexCount = std::size(indeces);
	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * indexCount;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indeces;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	device->CreateBuffer(&indexBufDesc, &indexData, &ib);
}

void CubeComponent::Draw(ID3D11Device* device, ID3D11DeviceContext* context) {


	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };

	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	context->DrawIndexed(24, 0, 0);
}

void CubeComponent::Translate(float x, float y, float z)
{
	for (int i = 0; i < 8; i++) {
		points[i].position.x += x;
		points[i].position.y += y;
		points[i].position.z += z;
	}
}

void CubeComponent::SetRotation(float rot)
{

	D3D11_MAPPED_SUBRESOURCE res = {};
	//context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto dataPtr = reinterpret_cast<float*>(res.pData);
	//context->Unmap(constantBuffer, 0);
}

void CubeComponent::SetColor(float r, float g, float b)
{
	auto color = Vector3(r, g, b);
	for (int i = 0; i < 8; i++) {
		points[i].normal = color;
	}
}

void CubeComponent::SetSize(float x, float y, float z)
{
	points[0].position = DirectX::XMFLOAT3(-x, z, -y);
	points[1].position = DirectX::XMFLOAT3(x, z, -y);
	points[2].position = DirectX::XMFLOAT3(x, z, y);
	points[3].position = DirectX::XMFLOAT3(-x, z, y);
	points[4].position = DirectX::XMFLOAT3(-x, -z, -y);
	points[5].position = DirectX::XMFLOAT3(x, -z, -y);
	points[6].position = DirectX::XMFLOAT3(x, -z, y);
	points[7].position = DirectX::XMFLOAT3(-x, -z, y);
}
