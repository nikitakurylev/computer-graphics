#pragma once
#include "QuadComponent.h";
#include <directxcollision.h>

class BoxComponent : public QuadComponent
{
public:
	BoxComponent(Game* game);
	DirectX::BoundingOrientedBox collider;
	void SetPosition(float x, float y, float z);
	void SetSize(float width, float height);
};

