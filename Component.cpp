#include "Component.h"

void Component::Start()
{
	gameObject->receive_transform_from_backend = true;
}

void Component::Update(float deltaTime)
{
}
