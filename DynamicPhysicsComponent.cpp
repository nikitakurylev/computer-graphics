#include "DynamicPhysicsComponent.h"

DynamicPhysicsComponent::DynamicPhysicsComponent() : PhysicsComponent()
{
	_bodyDef.bodyType = eDynamicBody;
}

void DynamicPhysicsComponent::Update(float deltaTime)
{
    auto transform = gameObject->GetTransform();
    auto position = _body->GetTransform().position;
    auto rotation = _body->GetQuaternion();
    transform->position = Vector3(position.v);
    transform->rotation = Quaternion(rotation.v);
}
