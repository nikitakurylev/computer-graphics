#include "KatamariComponent.h"
#include "BulletComponent.h"

KatamariComponent::KatamariComponent() : _bullets()
{
	speed = 1;
}

void KatamariComponent::Start() 
{
	for (GameObject* object : gameObject->GetGame()->GameObjects)
	{
		for (Component* component : object->GetComponents()) {
			auto bullet = dynamic_cast<BulletComponent*>(component);
			if (!bullet)
				continue;
			_bullets.push_back(bullet);
		}
	}
	auto transform = gameObject->GetTransform();
	transform->position.y = collider.Radius = 1.0f;
	transform->immovable = true;
}

void KatamariComponent::Update(float deltaTime)
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
	auto bulletsCount = _bullets.size();
	if (game->Input->IsKeyDown(Keys::E)) {
		if (!shootButtonDown) {
			game->Audio.PlaySoundClip("shoot.wav");
			_bullets[currentBullet % bulletsCount]->gameObject->GetTransform()->position = transform->position;
			_bullets[currentBullet % bulletsCount]->velocity = -right * 10;
			currentBullet = (currentBullet + 1) % bulletsCount;
			shootButtonDown = true;
		}
	}
	else if (game->Input->IsKeyDown(Keys::R)) {
		if (!shootButtonDown) {
			_bullets[currentBullet % bulletsCount]->gameObject->GetTransform()->position = transform->position;
			_bullets[currentBullet % bulletsCount]->velocity = Vector3::Zero;
			currentBullet = (currentBullet + 1) % bulletsCount;
			shootButtonDown = true;
		}
	}
	else
	{
		shootButtonDown = false;
	}

	acceleration = Vector3(acceleration.x, 0, acceleration.z) * speed;

	if (isGrounded && game->Input->IsKeyDown(Keys::Space)) {
		isGrounded = false;
		acceleration.y += 20;
	}

	velocity.x *= 0.7f;
	velocity.z *= 0.7f;
	velocity.y -= 30 * deltaTime;

	velocity += acceleration;

	transform->rotation *= Quaternion::CreateFromAxisAngle(Vector3::Right, velocity.z * deltaTime)
		* Quaternion::CreateFromAxisAngle(Vector3::Forward, velocity.x * deltaTime);

	transform->position += velocity * deltaTime;
	if (transform->position.y < collider.Radius) {
		transform->position.y = collider.Radius;
		velocity.y = 0;
		isGrounded = true;
	}
	game->cam_pos = transform->position;

	collider.Center = transform->position;

	Component::Update(deltaTime);

	for (GameObject* object : game->GameObjects)
	{
		auto otherTransform = object->GetTransform();
		auto otherWorldPosition = otherTransform->GetMatrix().Translation();
		if (object == gameObject || otherTransform->immovable || !collider.Contains(otherWorldPosition + Vector3::Up))
			continue;
		auto newMatrix = otherTransform->GetMatrix() *
			(Matrix::CreateFromQuaternion(transform->rotation) * Matrix::CreateTranslation(transform->position)).Invert();
		newMatrix.Decompose(otherTransform->scale, otherTransform->rotation, otherTransform->position);

		otherTransform->immovable = true;
		const float growth = 0.1f;
		speed += growth;
		collider.Radius += growth / 2;
		transform->position.y += growth / 2;
		transform->scale = Vector3::One * max(collider.Radius / 2.0f, 1.0f);
		otherTransform->parent = transform;
	}
}
