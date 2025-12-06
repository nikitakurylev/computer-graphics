#pragma once
#include <vector>
#include "Transform.h"
#include "Game.h"
class Component;

class GameObject
{
public:
	GameObject(Game* game);
	void Start();
	void Update(float deltaTime);
	void AddComponent(Component* component);
	void RemoveComponent(Component* component);
	const std::vector<Component*>& GetComponents() const { return components; }
	Transform* GetTransform() { return &transform; }
	Game* GetGame() const { return game; }
private:
	Transform transform;
	std::vector<Component*> components;
	Game* game;
};

