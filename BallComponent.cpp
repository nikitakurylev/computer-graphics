#include "BallComponent.h"
#include "BoxComponent.h"

BallComponent::BallComponent(Game* game) : BoxComponent(game)
{
}

void BallComponent::Update()
{
	for (GameComponent* gameComponent : game->Components)
	{
		auto* derived = dynamic_cast<BoxComponent*>(gameComponent);
		if (derived) {
			if (derived->collider.Intersects(collider)) {
				if (Velocity.x > 0) {
					if (derived->offsetColor.offset.x > offsetColor.offset.x)
						Velocity.x = -Velocity.x;
				}
				else if (Velocity.x < 0) {
					if (derived->offsetColor.offset.x < offsetColor.offset.x)
						Velocity.x = -Velocity.x;
				}
			}
		}
	}
	SetPosition(offsetColor.offset.x + Velocity.x, offsetColor.offset.y + Velocity.y, offsetColor.offset.z + Velocity.z);

}
