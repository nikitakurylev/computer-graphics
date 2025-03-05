#include "BallComponent.h"
#include "BoxComponent.h"
#include "PaddleComponent.h"
#include <iostream>

BallComponent::BallComponent(Game* game) : BoxComponent(game)
{
}

void BallComponent::Update()
{
	static int playerScore;
	static int enemyScore;
	for (GameComponent* gameComponent : game->Components)
	{
		if (gameComponent == this)
			continue;

		auto* box = dynamic_cast<BoxComponent*>(gameComponent);
		if (!box)
			continue;

		if (!box->collider.Intersects(collider))
			continue;
		auto* paddle = dynamic_cast<PaddleComponent*>(gameComponent);

		if (paddle) { 
			Velocity.y += 0.02f * (offsetColor.offset.y - paddle->offsetColor.offset.y) / paddle->collider.Extents.y;
			if (Velocity.x > 0) {
				Velocity.x = -Velocity.x;
			}
			else {
				Velocity.x = -Velocity.x;
			}

			Velocity.x *= 1.1f;
			break;
		}

		if (Velocity.y > 0) {
			Velocity.y = -Velocity.y;
		}
		else if (Velocity.y < 0) {
			Velocity.y = -Velocity.y;
		}
		break;
	}

	if (offsetColor.offset.x < -1.1) {
		SetPosition(0, 0, 0);
		Velocity.y = 0.01f;
		Velocity.x = 0.01f;
		enemyScore++;
		std::cout << playerScore << " " << enemyScore << std::endl;
	}
	else if (offsetColor.offset.x > 1.1) {
		SetPosition(0, 0, 0);
		Velocity.y = 0.01f;
		Velocity.x = 0.01f;
		playerScore++;
		std::cout << playerScore << " " << enemyScore << std::endl;
	}

	SetPosition(offsetColor.offset.x + Velocity.x, offsetColor.offset.y + Velocity.y, offsetColor.offset.z + Velocity.z);
}
