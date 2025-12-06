#pragma once
#include "ModelComponent.h"
#include "SphereComponent.h"
class KatamariComponent : public ModelComponent
{
public:
	KatamariComponent(ModelLoader* model, SphereComponent* bullets[10]);
	void Start() override;
	void Update(float deltaTime) override;
private:
	SphereComponent** _bullets;
	BoundingSphere collider;
	Vector3 velocity;
	int currentBullet;
	bool shootButtonDown;
	bool isGrounded;
	float speed;
};

