#include "KatamariComponent.h"

KatamariComponent::KatamariComponent(Game* game, std::string fileName) : ModelComponent(game, fileName)
{
}

void KatamariComponent::Update(float deltaTime)
{
	auto velocity = Vector3();
	auto camera_matrix = game->GetCameraMatrix();
	auto right = camera_matrix.Right();
	right.y = 0;
	right.Normalize();
	auto forward = camera_matrix.Forward();
	forward.y = 0;
	forward.Normalize();
	
	if (game->Input->IsKeyDown(Keys::W)) {
		rotation *= Quaternion::CreateFromAxisAngle(-forward, deltaTime);
		velocity -= right;
	}
	if (game->Input->IsKeyDown(Keys::S)) {
		rotation *= Quaternion::CreateFromAxisAngle(forward, deltaTime);
		velocity += right;
	}
	if (game->Input->IsKeyDown(Keys::D)) {
		rotation *= Quaternion::CreateFromAxisAngle(-right, deltaTime);
		velocity += forward;
	}
	if (game->Input->IsKeyDown(Keys::A)) {
		rotation *= Quaternion::CreateFromAxisAngle(right, deltaTime);
		velocity -= forward;
	}

	velocity = Vector3(velocity.x, 0, velocity.z) * deltaTime;

	position += velocity;
	game->cam_pos = position;
	UpdateWorldMatrix();
}
