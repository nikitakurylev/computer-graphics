#include "PlayerPaddleComponent.h"

PlayerPaddleComponent::PlayerPaddleComponent(Game* game) : PaddleComponent(game)
{
}

void PlayerPaddleComponent::Update()
{
	if (game->Input->IsKeyDown(Keys::Up))
		SetPosition(offsetColor.offset.x, offsetColor.offset.y + 0.01f, offsetColor.offset.z);
	if (game->Input->IsKeyDown(Keys::Down))
		SetPosition(offsetColor.offset.x, offsetColor.offset.y - 0.01f, offsetColor.offset.z);
}
