#include "GameObject.h"
#include "Component.h"

GameObject::GameObject(int32_t uid, Game* game, ScriptingTransformComponent* scriptingTransformComponent) : uid_(uid), game(game), scripting_transform_component(scriptingTransformComponent)
{
	transform = Transform();
	receive_transform_from_backend = true;
	send_transform_to_backend = false;
}

GameObject::~GameObject()
{
	for (auto scriptingComponent : scripting_components)
	{
		delete scriptingComponent;
	}

	delete scripting_transform_component;
}

void GameObject::Start()
{
	if (receive_transform_from_backend)
	{
		transform.position = scripting_transform_component->GetPosition();
		transform.scale = scripting_transform_component->GetScale();
	}

	transform.Update();
	
	for (Component* component : components)
	{
		component->Start();
	}

	for (ScriptingComponent* scriptingComponent : scripting_components)
	{
		scriptingComponent->Start();
	}

	if (send_transform_to_backend)
	{
		scripting_transform_component->UpdateTransform(transform.position, transform.scale);
	}
}

void GameObject::Update(float deltaTime)
{
	if (receive_transform_from_backend)
	{
		transform.position = scripting_transform_component->GetPosition();
		transform.scale = scripting_transform_component->GetScale();
	}

	transform.Update();

	for (Component* component : components)
	{
		component->Update(deltaTime);
	}

	for (ScriptingComponent* scriptingComponent : scripting_components)
	{
		scriptingComponent->Update(deltaTime);
	}

	if (send_transform_to_backend)
	{
		scripting_transform_component->UpdateTransform(transform.position, transform.scale);
	}
}

void GameObject::AddComponent(Component* component)
{
	if (component->gameObject != nullptr)
		component->gameObject->RemoveComponent(component);
	
	component->gameObject = this;
	components.push_back(component);
}

void GameObject::AddScriptingComponent(ScriptingComponent* scriptingComponent)
{
	scripting_components.push_back(scriptingComponent);
}

void GameObject::RemoveComponent(Component* component)
{
	if (component->gameObject == this)
		components.erase(std::remove(components.begin(), components.end(), component), components.end());
}
