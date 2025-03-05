#pragma once
#include "PaddleComponent.h"
#include "BallComponent.h"
class EnemyPaddleComponent : public PaddleComponent
{
public:
	EnemyPaddleComponent(Game* game, BallComponent* ball);
	void Update() override;
private:
	BallComponent* Ball;
};

