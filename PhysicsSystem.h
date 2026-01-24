#pragma once
#include "qu3e/q3.h"

class PhysicsSystem
{
public:
	PhysicsSystem();
	q3Body* CreateBody(q3BodyDef bodyDef);
	void Update(float deltaTime);
private:
	q3Scene _scene;
	float _time;
};

