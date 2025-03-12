#include "CubeComponent.h"
#include <directxmath.h>
#include <iostream>
#include "GameComponent.h"
#include "Game.h"

CubeComponent::CubeComponent(Game* game) : GameComponent(game)
{
	SetSize(0.5f, 0.5f, 0.5f);
	for (int i = 0; i < 8; i++)
		points[i].color = color * (16 - i) / 16.0f;
}

void CubeComponent::Initialize(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader)
{
	VertexShader = vertexShader;
	PixelShader = pixelShader;

	points[0].texCoord = DirectX::XMFLOAT2(1, 1);
	points[1].texCoord = DirectX::XMFLOAT2(-1, -1);
	points[2].texCoord = DirectX::XMFLOAT2(-1, 1);
	points[3].texCoord = DirectX::XMFLOAT2(1, -1);
}

void CubeComponent::Draw() {

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

	ID3D11Buffer* vb;
	game->Device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

	int indeces[] = { 
		3,1,0,
		2,1,3,

		0,5,4,
		1,5,0,

		3,4,7,
		0,4,3,

		1,6,5,
		2,6,1,

		2,7,6,
		3,7,2,

		6,4,5,
		7,4,6,
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

	ID3D11Buffer* ib;
	game->Device->CreateBuffer(&indexBufDesc, &indexData, &ib);

	UINT strides[] = { 40 };
	UINT offsets[] = { 0 };

	game->Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game->Context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game->Context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game->Context->VSSetShader(VertexShader, nullptr, 0);
	game->Context->PSSetShader(PixelShader, nullptr, 0);
	game->Context->DrawIndexed(indexCount, 0, 0);
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
	offsetColor.rotation.w = rot;

	D3D11_MAPPED_SUBRESOURCE res = {};
	//game->Context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, &offsetColor, sizeof(OffsetColor));
	//game->Context->Unmap(constantBuffer, 0);
}

void CubeComponent::SetColors(int index, float r, float g, float b)
{
	points[index].color = DirectX::XMFLOAT4(r, g, b, 1.0f);
}

void CubeComponent::SetSize(float x, float y, float z)
{
	points[0].position = DirectX::XMFLOAT4(-x, z, -y, 0);
	points[1].position = DirectX::XMFLOAT4(x, z, -y, 0);
	points[2].position = DirectX::XMFLOAT4(x, z, y, 0);
	points[3].position = DirectX::XMFLOAT4(-x, z, y, 0);
	points[4].position = DirectX::XMFLOAT4(-x, -z, -y, 0);
	points[5].position = DirectX::XMFLOAT4(x, -z, -y, 0);
	points[6].position = DirectX::XMFLOAT4(x, -z, y, 0);
	points[7].position = DirectX::XMFLOAT4(-x, -z, y, 0);
}
