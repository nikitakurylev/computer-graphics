#include "KatamariComponent.h"

KatamariComponent::KatamariComponent(Game* game, ModelLoader* model) : ModelComponent(game, model)
{
	position.y = collider.Radius = 1.0f;
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

	auto deltaSpeed = deltaTime * speed;
	
	if (game->Input->IsKeyDown(Keys::W)) {
		rotation *= Quaternion::CreateFromAxisAngle(-forward, deltaSpeed);
		velocity -= right;
	}
	if (game->Input->IsKeyDown(Keys::S)) {
		rotation *= Quaternion::CreateFromAxisAngle(forward, deltaSpeed);
		velocity += right;
	}
	if (game->Input->IsKeyDown(Keys::D)) {
		rotation *= Quaternion::CreateFromAxisAngle(-right, deltaSpeed);
		velocity += forward;
	}
	if (game->Input->IsKeyDown(Keys::A)) {
		rotation *= Quaternion::CreateFromAxisAngle(right, deltaSpeed);
		velocity -= forward;
	}

	velocity = Vector3(velocity.x, 0, velocity.z) * deltaSpeed;

	position += velocity;
	game->cam_pos = position;

	collider.Center = position;

	GameComponent::Update(deltaTime);

	for (GameComponent* object : game->Components)
	{
		if (object == this || object->immovable || !collider.Contains(object->position + Vector3::Up))
			continue;
		auto newMatrix = object->world_matrix * 
			(Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position)).Invert();
		newMatrix.Decompose(object->scale, object->rotation, object->position);

		object->immovable = true;
		const float growth = 0.1f;
		speed += growth;
		collider.Radius += growth / 2;
		position.y += growth / 2;
		scale = Vector3::One * max(collider.Radius / 2.0f, 1.0f);
		object->parent = this;
	}
}
