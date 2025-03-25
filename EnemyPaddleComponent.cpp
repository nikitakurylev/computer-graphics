#include "EnemyPaddleComponent.h"

EnemyPaddleComponent::EnemyPaddleComponent(Game* game, BallComponent* ball) : PaddleComponent(game), Ball(ball)
{
}

void EnemyPaddleComponent::Update(float deltaTime)
{
	if(Ball->offsetColor.offset.y > offsetColor.offset.y)
		SetPosition(offsetColor.offset.x, offsetColor.offset.y + fminf(0.009f, Ball->offsetColor.offset.y - offsetColor.offset.y), offsetColor.offset.z);
	if (Ball->offsetColor.offset.y < offsetColor.offset.y)
		SetPosition(offsetColor.offset.x, offsetColor.offset.y - fminf(0.009f,  offsetColor.offset.y - Ball->offsetColor.offset.y), offsetColor.offset.z);
}


