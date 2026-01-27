#pragma once
#include <vector>
#include "Transform.h"
#include "Game.h"
#include "ScriptingComponent.hpp"
#include "ScriptingTransformComponent.hpp"
#include "ScriptingEngine.h"

class Component;

class GameObject
{
public:
	GameObject(int32_t uid, Game* game, ScriptingTransformComponent* scriptingTransformComponent);
	~GameObject();
	void Start();
	void Update(float deltaTime);
	void AddComponent(Component* component);
	void AddScriptingComponent(ScriptingComponent* scriptingComponent);
	void RemoveComponent(Component* component);
	const std::vector<Component*>& GetComponents() const { return components; }
	const std::vector<ScriptingComponent*>& GetScriptingComponents() const { return scripting_components; }
	Transform* GetTransform() { return &transform; }
	ScriptingTransformComponent* GetScriptingTransformComponent() { return scripting_transform_component; }
	Game* GetGame() const { return game; }
	bool receive_transform_from_backend;
	bool send_transform_to_backend;
private:
	Transform transform;
	ScriptingTransformComponent* scripting_transform_component;
	std::vector<Component*> components;
	std::vector<ScriptingComponent*> scripting_components;
	Game* game;
	int32_t uid_;
};

