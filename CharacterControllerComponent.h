#pragma once
#include "DynamicPhysicsComponent.h"
#include "SimpleMath.h"
class CharacterControllerComponent : public DynamicPhysicsComponent
{
public:
	CharacterControllerComponent();
	void Update(float deltaTime) override;
private:
	bool isGrounded;
	float speed;
	DirectX::BoundingSphere collider;
};

