#include "EnemyPaddleComponent.h"

EnemyPaddleComponent::EnemyPaddleComponent(Game* game, BallComponent* ball) : PaddleComponent(game), Ball(ball)
{
}

void EnemyPaddleComponent::Update()
{
	if(Ball->offsetColor.offset.y > offsetColor.offset.y)
		SetPosition(offsetColor.offset.x, offsetColor.offset.y + 0.009f, offsetColor.offset.z);
	if (Ball->offsetColor.offset.y < offsetColor.offset.y)
		SetPosition(offsetColor.offset.x, offsetColor.offset.y - 0.009f, offsetColor.offset.z);
}


