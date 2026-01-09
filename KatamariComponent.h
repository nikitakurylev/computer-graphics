#pragma once
#include "Component.h"
#include "BulletComponent.h"
class KatamariComponent : public Component
{
public:
	KatamariComponent(BulletComponent* bullets[10]);
	void Start() override;
	void Update(float deltaTime) override;
private:
	BulletComponent** _bullets;
	BoundingSphere collider;
	Vector3 velocity;
	int currentBullet;
	bool shootButtonDown;
	bool isGrounded;
	float speed;
};

