#include "BulletComponent.h"
#include <directxmath.h>
#include "Component.h"
#include "Game.h"
#include "GameObject.h"
#include "Transform.h"

using namespace DirectX::SimpleMath;

// ============================================================
BulletComponent::BulletComponent() {}

// ============================================================
void BulletComponent::Spawn(const Vector3& origin, const Vector3& vel, float maxRange)
{
	// Гарантируем что backend не перезапишет позицию после Spawn
	gameObject->receive_transform_from_backend = false;
	gameObject->send_transform_to_backend = false;

	auto transform = gameObject->GetTransform();
	transform->position = origin;
	spawnPos_ = origin;
	velocity = vel;
	maxRange_ = maxRange;
	active = true;
}

// ============================================================
void BulletComponent::Start()
{
	// Отключаем синхронизацию с Mono-бэкендом:
	// receive_transform_from_backend = true (которое ставит базовый Component::Start)
	// каждый кадр в GameObject::Update перезаписывает позицию значением из C# (-1e7),
	// из-за чего шарики никуда не летят.
	gameObject->receive_transform_from_backend = false;
	gameObject->send_transform_to_backend = false;

	auto transform = gameObject->GetTransform();
	transform->position.y = kInactiveY;
	active = false;
}

// ============================================================
void BulletComponent::Update(float deltaTime)
{
	if (!active) return;

	auto transform = gameObject->GetTransform();
	transform->position += velocity * deltaTime;
	collider.Center = transform->position;

	// Исчезаем если пролетели дальше maxRange_
	float dist = Vector3::Distance(transform->position, spawnPos_);
	if (dist >= maxRange_)
	{
		active = false;
		transform->position.y = kInactiveY;
		velocity = Vector3::Zero;
	}

	Component::Update(deltaTime);
}

// ============================================================
void BulletComponent::Draw(ID3D11Device* /*device*/, ID3D11DeviceContext* context)
{
	if (!active) return;

	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };
	context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	context->PSSetShaderResources(0, 1, &texture);
	context->DrawIndexed(2400, 0, 0);
}

// ============================================================
void BulletComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* /*context*/)
{
	constexpr float thau = 6.28318530718f;
	for (auto j = 0; j <= 20; j++)
	{
		const auto height = static_cast<float>(j) / 20.0f;
		const auto t = sqrtf(height * (1.0f - height));

		for (auto i = 0; i < 20; i++)
		{
			float s, c;
			DirectX::XMScalarSinCos(&s, &c, thau * static_cast<float>(i) / 20.0f);

			const auto position = DirectX::XMFLOAT3(c * t, height - 0.5f, s * t);
			points[j * 20 + i].position = position;

			auto norm = Vector3(position.x, position.y, position.z);
			norm.Normalize();
			points[j * 20 + i].normal = -norm;
			points[j * 20 + i].texCoord = { 0, 0 };
			points[j * 20 + i].tangent = { 0, 0, 0 };
			points[j * 20 + i].bitangent = { 0, 0, 0 };
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

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.ByteWidth = sizeof(Vertex) * (UINT)std::size(points);
	D3D11_SUBRESOURCE_DATA vd = {};
	vd.pSysMem = points;
	device->CreateBuffer(&vbd, &vd, &vb);

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.ByteWidth = sizeof(int) * (UINT)std::size(indeces);
	D3D11_SUBRESOURCE_DATA id = {};
	id.pSysMem = indeces;
	device->CreateBuffer(&ibd, &id, &ib);

	collider.Radius = 0.15f;
}