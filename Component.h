#pragma once
#include "GameObject.h"
class Component
{
public:
	GameObject* gameObject = nullptr;
	virtual void Start();
	virtual void Update(float deltaTime);
};

