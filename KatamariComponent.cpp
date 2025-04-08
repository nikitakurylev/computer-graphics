#include "KatamariComponent.h"
#include "SphereComponent.h"

KatamariComponent::KatamariComponent(Game* game, ModelLoader* model, SphereComponent* bullets[10]) : ModelComponent(game, model)
{
	position.y = collider.Radius = 1.0f;
	_bullets = bullets;
	immovable = true;
}

void KatamariComponent::Update(float deltaTime)
{
	auto acceleration = Vector3();
	auto camera_matrix = game->GetCameraMatrix();
	auto right = camera_matrix.Right();
	right.y = 0;
	right.Normalize();
	auto forward = camera_matrix.Forward();
	forward.y = 0;
	forward.Normalize();

	auto deltaSpeed = deltaTime * speed;
	
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
	
	if (game->Input->IsKeyDown(Keys::E)) {
		if (!shootButtonDown) {
			_bullets[currentBullet % 10]->position = position;
			_bullets[currentBullet % 10]->velocity = -right * 10;
			currentBullet = (currentBullet + 1) % 10;
			shootButtonDown = true;
		}
	}
	else
	{
		shootButtonDown = false;
	}

	acceleration = Vector3(acceleration.x, 0, acceleration.z) * deltaSpeed + Vector3::Down * deltaTime;

	velocity.x *= 0.7f;
	velocity.z *= 0.7f;

	velocity += acceleration;

	rotation *= Quaternion::CreateFromAxisAngle(Vector3::Right, velocity.z)
		* Quaternion::CreateFromAxisAngle(Vector3::Forward, velocity.x);

	position += velocity;
	if (position.y <= collider.Radius) {
		position.y = collider.Radius;
		velocity.y = 0;
		if (game->Input->IsKeyDown(Keys::Space)) {
			velocity += Vector3::Up * 0.5f;
		}
	}
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
