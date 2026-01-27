#include "BulletComponent.h"
#include <directxmath.h>
#include <iostream>
#include "Component.h"
#include "Game.h"

BulletComponent::BulletComponent()
{
}

void BulletComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
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

			points[j * 20 + i].position = DirectX::XMFLOAT3(position.x, position.y, position.z);
			//points[j * 20 + i].color = DirectX::XMFLOAT4(position.x + .1f, position.y + .7f, position.z + .3f, 1.0f);
			//points[j * 20 + i].color = DirectX::XMFLOAT4(0, i%2, 0, 1.0f);
			//points[j * 20 + i].color = DirectX::XMFLOAT4(j%2, i%2, j * 0.05f, 1.0f);
			//points[j * 20 + i].color = DirectX::XMFLOAT4((j + i) % 2 * color.x, color.y, color.z * 0.5f + j * 0.025f, 1.0f);
			auto norm = Vector3(position.x, position.y, position.z);
			norm.Normalize();
			points[j * 20 + i].normal = -norm;
			points[j * 20 + i].texCoord = Vector2(0, 0);
			points[j * 20 + i].tangent = Vector3::Zero;
			points[j * 20 + i].bitangent = Vector3::Zero;
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

void BulletComponent::Draw(ID3D11Device* device, ID3D11DeviceContext* context) 
{
	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };
	context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	context->PSSetShaderResources(0, 1, &texture);
	context->DrawIndexed(2400, 0, 0);
}

void BulletComponent::Start() 
{
	auto transform = gameObject->GetTransform();
	transform->immovable = true;
	transform->position.y = -10000000000;
}

void BulletComponent::Update(float deltaTime)
{
	auto transform = gameObject->GetTransform();
	transform->position += velocity * deltaTime;zxfwefelas0pdfklweq[231432458623548045]
	collider.Center = transform->position;

	for (GameObject* object : gameObject->GetGame()->GameObjects)
	{
		if (object == gameObject || object->GetTransform()->immovable || !collider.Contains(object->GetTransform()->position + Vector3::Up))
			continue;
		object->GetTransform()->position.y = -10000000000;
		transform->position.y = -10000000000;
		velocity = Vector3();
	}

	Component::Update(deltaTime);
}
