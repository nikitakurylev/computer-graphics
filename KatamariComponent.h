#pragma once
#include "ModelComponent.h"
class KatamariComponent : public ModelComponent
{
public:
	KatamariComponent(Game* game, std::string fileName);
	void Update(float deltaTime) override;
};

