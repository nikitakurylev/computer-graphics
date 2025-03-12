#include "SphereComponent.h"
#include <directxmath.h>
#include <iostream>
#include "GameComponent.h"
#include "Game.h"

SphereComponent::SphereComponent(Game* game) : GameComponent(game)
{
	constexpr float thau = 6.28318530718f;
	for (auto j = 0; j <= 20; j++)
	{
		const auto height = static_cast<float>(j) / 20.0f;
		const auto t = sqrtf(height * (1.0f - height));

		for (auto i = 0; i < 20; i++)
		{
			float sin, cos;

			DirectX::XMScalarSinCos(&sin, &cos, thau * static_cast<float>(i) / 20.0f);

			const auto position = DirectX::XMFLOAT3(cos * t, height - 0.5f, sin * t);

			points[j * 20 + i].position = DirectX::XMFLOAT4(position.x, position.y, position.z, 1.0f);
			//points[j * 20 + i].color = DirectX::XMFLOAT4(position.x + .1f, position.y + .7f, position.z + .3f, 1.0f);
			//points[j * 20 + i].color = DirectX::XMFLOAT4(0, i%2, 0, 1.0f);
			//points[j * 20 + i].color = DirectX::XMFLOAT4(j%2, i%2, j * 0.05f, 1.0f);
			points[j * 20 + i].color = DirectX::XMFLOAT4((j + i) % 2 * color.x, color.y, color.z * 0.5f + j * 0.025f, 1.0f);
		}
	}

	for (auto j = 0; j < 20; j++)
		for (auto i = 0; i < 20; i++)
		{
			const int index = j * 20 + i;

			indeces[6 * index] = index;
			indeces[6 * index + 1] = index + 20 - 1;
			indeces[6 * index + 2] = index + 20;
			indeces[6 * index + 3] = index + 20;
			indeces[6 * index + 4] = index + 1;
			indeces[6 * index + 5] = index;
		}
}

void SphereComponent::Initialize(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader)
{
	VertexShader = vertexShader;
	PixelShader = pixelShader;

	points[0].texCoord = DirectX::XMFLOAT2(1, 1);
	points[1].texCoord = DirectX::XMFLOAT2(-1, -1);
	points[2].texCoord = DirectX::XMFLOAT2(-1, 1);
	points[3].texCoord = DirectX::XMFLOAT2(1, -1);

}

void SphereComponent::Draw() {

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

void SphereComponent::Update()
{
	rotation.y += 0.01f;
	UpdateWorldMatrix();
}

void SphereComponent::SetRotation(float rot)
{
}

void SphereComponent::SetColors(int index, float r, float g, float b)
{
	points[index].color = DirectX::XMFLOAT4(r, g, b, 1.0f);
}
