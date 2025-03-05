#pragma once
#include "PaddleComponent.h"
class PlayerPaddleComponent : public PaddleComponent
{
public:
	PlayerPaddleComponent(Game* game);
	void Update() override;
};

