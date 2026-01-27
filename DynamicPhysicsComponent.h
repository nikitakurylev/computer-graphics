#pragma once
#include "PhysicsComponent.h"
class DynamicPhysicsComponent : public PhysicsComponent
{
public:
	DynamicPhysicsComponent();
	void Update(float deltaTime) override;
};

