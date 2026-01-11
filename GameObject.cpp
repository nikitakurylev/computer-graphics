#include "GameObject.h"
#include "Component.h"

GameObject::GameObject(Game* game) : game(game)
{
	transform = Transform();
}

void GameObject::Start()
{
	transform.Update();
	
	for (Component* component : components)
	{
		component->Start();
	}
}


void GameObject::Update(float deltaTime)
{
	transform.Update();

	for (Component* component : components)
	{
		component->Update(deltaTime);
	}
}

void GameObject::AddComponent(Component* component)
{
	if (component->gameObject != nullptr)
		component->gameObject->RemoveComponent(component);
	
	component->gameObject = this;
	components.push_back(component);
}

void GameObject::RemoveComponent(Component* component)
{
	if (component->gameObject == this)
		components.erase(std::remove(components.begin(), components.end(), component), components.end());
}
