#include "BallComponent.h"
#include "BoxComponent.h"
#include "PaddleComponent.h"
#include <iostream>
#include <d3d11.h>
#include <DirectXMath.h>

BallComponent::BallComponent(Game* game) : BoxComponent(game)
{
	ShaderPath = L"./Shaders/Ball.hlsl";
}

void BallComponent::Update(float deltaTime)
{
	static int playerScore;
	static int enemyScore;

	if (offsetColor.offset.x < -1.1) {
		SetPosition(0, 0, 0);
		Velocity.y = 0;
		Velocity.x = 0.01f;
		enemyScore++;
		std::cout << playerScore << " " << enemyScore << std::endl;
	}
	else if (offsetColor.offset.x > 1.1) {
		SetPosition(0, 0, 0);
		Velocity.y = 0;
		Velocity.x = 0.01f;
		playerScore++;
		std::cout << playerScore << " " << enemyScore << std::endl;
	}

	for (GameComponent* gameComponent : game->Components)
	{
		auto* ball = dynamic_cast<BallComponent*>(gameComponent);
		if (ball)
			continue;

		auto* box = dynamic_cast<BoxComponent*>(gameComponent);
		if (!box)
			continue;
		float dist;
		DirectX::XMVECTORF32 origin = { offsetColor.offset.x, offsetColor.offset.y, offsetColor.offset.z, 0 };
		DirectX::XMVECTORF32 direction = { Velocity.x, Velocity.y, Velocity.z, 0 };
		if (!box->collider.Intersects(collider) &&
			(!box->collider.Intersects(origin, XMVector3Normalize(direction), dist) ||
			dist * dist >= Velocity.x * Velocity.x + Velocity.y * Velocity.y + Velocity.z * Velocity.z))
			continue;

		auto* paddle = dynamic_cast<PaddleComponent*>(gameComponent);

		if (paddle) { 
			Velocity.y += 0.01f * (offsetColor.offset.y - paddle->offsetColor.offset.y) / paddle->collider.Extents.y;
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

	SetPosition(offsetColor.offset.x + Velocity.x, offsetColor.offset.y + Velocity.y, offsetColor.offset.z + Velocity.z);
}
