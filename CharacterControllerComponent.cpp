#include "CharacterControllerComponent.h"

CharacterControllerComponent::CharacterControllerComponent() : DynamicPhysicsComponent()
{
	_bodyDef.lockAxisX = true;
	_bodyDef.lockAxisZ = true;
	_bodyDef.lockAxisY = true;
	_bodyDef.gravityScale = 2;
	collider.Radius = 0.5f;
	speed = 2;
}

void CharacterControllerComponent::Start()
{
	DynamicPhysicsComponent::Start();
	auto transform = gameObject->GetTransform();
	spawnPoint = transform->position;
	auto game = gameObject->GetGame();
	for (GameObject* object : game->GameObjects) {
		auto otherTransform = object->GetTransform();
		auto otherWorldPosition = otherTransform->GetMatrix().Translation();
		if (otherTransform->parent != transform)
			continue;
		for (Component* component : object->GetComponents()) {
			animation = dynamic_cast<AnimationComponent*>(component);
			if (animation)
				break;
		}
		break;
	}

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

	if (isGrounded && game->Input->IsKeyDown(Keys::Space)) {
		isGrounded = false;
		game->Audio.PlaySoundClip("jump.wav");
		velocity.y = 7;
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
		if (object == gameObject || !collider.Contains(otherWorldPosition))
			continue;

		for (ScriptingComponent* component : object->GetScriptingComponents()) {
			game->Audio.PlaySoundClip("collect.wav");
		}
	}
	auto oldRotation = transform->rotation;

	DynamicPhysicsComponent::Update(deltaTime);

	auto isWalking = acceleration.LengthSquared() > 0;
	animation->playing = isWalking;
	if (isWalking)
		transform->rotation = Quaternion::Lerp(oldRotation, Quaternion::LookRotation(acceleration, Vector3::Up), deltaTime * 10.0f);
	else
		transform->rotation = oldRotation;

	isGrounded = (!game->Input->IsKeyDown(Keys::Space)) && game->Physics.Raycast(transform->position + Vector3(0, -0.4001, 0), Vector3::Down, 0.0001f);

	if (transform->position.y < 0) {
		_body->SetTransform(q3Vec3(spawnPoint.x, spawnPoint.y, spawnPoint.z));
	}
}
