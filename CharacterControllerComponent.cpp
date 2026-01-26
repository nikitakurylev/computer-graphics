#include "CharacterControllerComponent.h"

CharacterControllerComponent::CharacterControllerComponent() : DynamicPhysicsComponent()
{
	_bodyDef.lockAxisX = true;
	_bodyDef.lockAxisZ = true;
	_bodyDef.lockAxisY = true;
	speed = 1;
}

void CharacterControllerComponent::Update(float deltaTime)
{
	auto acceleration = Vector3();
	auto game = gameObject->GetGame();
	auto camera_matrix = game->GetCameraMatrix();
	auto right = camera_matrix.Right();
	right.y = 0;
	right.Normalize();
	auto forward = camera_matrix.Forward();
	forward.y = 0;
	forward.Normalize();

	if (game->Input->IsKeyDown(Keys::W)) {
		acceleration -= right;
	}
	if (game->Input->IsKeyDown(Keys::S)) {
		acceleration += right;
	}
	if (game->Input->IsKeyDown(Keys::D)) {
		acceleration += forward;
	}
	if (game->Input->IsKeyDown(Keys::A)) {
		acceleration -= forward;
	}

	auto transform = gameObject->GetTransform();

	acceleration = Vector3(acceleration.x, 0, acceleration.z) * speed;
	auto velocity = _body->GetLinearVelocity();

	if (/*isGrounded &&*/ game->Input->IsKeyDown(Keys::Space)) {
		//isGrounded = false;
		velocity.y = 5;
	}

	velocity.x = acceleration.x * speed;
	velocity.z = acceleration.z * speed;
	_body->SetLinearVelocity(velocity);

	game->cam_pos = transform->position;

	collider.Center = transform->position;

	Component::Update(deltaTime);

	for (GameObject* object : game->GameObjects)
	{
		auto otherTransform = object->GetTransform();
		auto otherWorldPosition = otherTransform->GetMatrix().Translation();
		if (object == gameObject || otherTransform->immovable || !collider.Contains(otherWorldPosition + Vector3::Up))
			continue;
	}
	
	DynamicPhysicsComponent::Update(deltaTime);
}
