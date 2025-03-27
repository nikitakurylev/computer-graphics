#pragma once
#include "ModelComponent.h"
class KatamariComponent : public ModelComponent
{
public:
	KatamariComponent(Game* game, ModelLoader* model);
	void Update(float deltaTime) override;
private:
	BoundingSphere collider;
};

