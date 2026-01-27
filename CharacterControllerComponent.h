#pragma once
#include "DynamicPhysicsComponent.h"
#include "SimpleMath.h"
#include "AnimationComponent.h"
class CharacterControllerComponent : public DynamicPhysicsComponent
{
public:
	CharacterControllerComponent();
	void Start() override;
	void Update(float deltaTime) override;
private:
	bool isGrounded;
	float speed;
	DirectX::BoundingSphere collider;
	AnimationComponent* animation;
	Vector3 spawnPoint;
};

