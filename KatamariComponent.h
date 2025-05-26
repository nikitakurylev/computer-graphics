#pragma once
#include "ModelComponent.h"
#include "SphereComponent.h"
class KatamariComponent : public ModelComponent
{
public:
	KatamariComponent(Game* game, ModelLoader* model, SphereComponent* bullets[10]);
	void Update(float deltaTime) override;
private:
	SphereComponent** _bullets;
	BoundingSphere collider;
	Vector3 velocity;
	int currentBullet;
	bool shootButtonDown;
	bool isGrounded;
};

