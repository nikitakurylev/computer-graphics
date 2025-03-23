#include "PlayerPaddleComponent.h"

PlayerPaddleComponent::PlayerPaddleComponent(Game* game) : PaddleComponent(game)
{
}

void PlayerPaddleComponent::Update(float deltaTime)
{
	if (game->Input->IsKeyDown(Keys::Up))
		SetPosition(offsetColor.offset.x, offsetColor.offset.y + 0.01f, offsetColor.offset.z);
	if (game->Input->IsKeyDown(Keys::Down))
		SetPosition(offsetColor.offset.x, offsetColor.offset.y - 0.01f, offsetColor.offset.z);

	if (game->Input->IsKeyDown(Keys::Right))
		SetBoxRotation(offsetColor.rotation.w + 0.01f);
	if (game->Input->IsKeyDown(Keys::Left))
		SetBoxRotation(offsetColor.rotation.w - 0.01f);
}
