#pragma once
#include "Component.h"
class PointLightComponent : public Component
{
public:
	PointLightComponent(Vector4 color, float radius);
	Vector4 color;
	float radius; 
};

