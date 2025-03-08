#include "BoxComponent.h"
#include "QuadComponent.h"
#include <directxcollision.h>

BoxComponent::BoxComponent(Game* game) : QuadComponent(game), collider(DirectX::BoundingOrientedBox(DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), DirectX::XMFLOAT4(0, 0, 0, 1.0f))){

}

void BoxComponent::SetPosition(float x, float y, float z)
{
	SetOffset(x, y, z);
	collider.Center = DirectX::XMFLOAT3(x, y, z);
}

void BoxComponent::SetSize(float width, float height)
{
	auto halfWidth = width * 0.5f;
	auto halfHeight = height * 0.5f;

	SetPositions(
		halfWidth, halfHeight, 0.5f,
		-halfWidth, -halfHeight, 0.5f,
		-halfWidth, halfHeight, 0.5f,
		halfWidth, -halfHeight, 0.5f
	);
	collider.Extents.x = halfWidth;
	collider.Extents.y = halfHeight;
}

void BoxComponent::SetBoxRotation(float rot)
{
	SetRotation(rot);
	DirectX::XMVECTORF32 axis = { 0, 0, 1 };
	DirectX::XMFLOAT4 result;
	DirectX::XMStoreFloat4(&result, DirectX::XMQuaternionRotationAxis(axis, rot));
	collider.Orientation = result;
}
