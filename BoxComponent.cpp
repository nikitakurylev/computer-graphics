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
