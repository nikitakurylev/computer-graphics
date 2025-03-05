#pragma once
#include "BoxComponent.h"
#include <d3d11.h>

class BallComponent : public BoxComponent
{
public:
	BallComponent(Game* game);
	DirectX::XMFLOAT3 Velocity;
	void Update() override;
};

