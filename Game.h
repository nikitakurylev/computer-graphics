#pragma once
#include <vector>
#include "DisplayWin32.h"
#include "InputDevice.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <chrono>
#include "SimpleMath.h"
#include "CascadeData.h"
#include "RenderingSystem.h"
#include "ScriptingEngine.h"
#include "PhysicsSystem.h"
#include "AudioSystem.h"

using namespace DirectX::SimpleMath;
class GameObject;
class CubeComponent;
class BulletComponent;

struct ConstantBuffer
{
	Matrix World;
	Matrix ViewProjection;
	Vector4 ViewPosition;
	Matrix InverseProjectionView;
	Matrix ViewInv;
	Matrix ProjInv;
	Matrix View;
	Matrix Projection;
};

class Game
{
public:
	Game(DisplayWin32* display, InputDevice* input, RenderingSystem* render, ScriptingEngine* scriptingEngine);
	~Game();
	std::vector<GameObject*> GameObjects;
	DisplayWin32* Display;
	InputDevice* Input;
	RenderingSystem* Render;
	PhysicsSystem Physics;
	AudioSystem Audio;

	// ѕозици€ и ориентаци€ камеры (публичные Ч нужны компонентам)
	Vector3 cam_pos;
	Vector3 cam_world;   // дл€ системы теней (=cam_pos в fps-режиме)
	Vector3 cam_rot;     // yaw/pitch/roll камеры

	void Initialize();
	void Run();
	void Update(float deltaTime);
	void RenderDebugUI();
	Matrix GetCameraMatrix();

	// —трельба: регистрирует шарик из пула
	void RegisterProjectile(BulletComponent* bullet);

	Matrix view_matrix;
	Matrix projection_matrix;
	ScriptingEngine* scripting_engine;

private:
	// ¬нутренн€€ логика стрельбы
	void UpdateShooting(float deltaTime);

	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float TotalTime;

	Matrix light_view_proj[4];

	bool ortho;
	float distance;

	// ѕул шариков (заполн€етс€ из MySuper3DApp после Initialize)
	std::vector<BulletComponent*> projectilePool_;
	int  nextProjectile_ = 0;
	bool lmbWasDown_ = false;  // edge-detection кнопки X

	static constexpr float kProjectileSpeed = 8.0f;   // единиц/с
	static constexpr float kProjectileRange = 150.0f; // единиц Ч дальность до авто-удалени€

	Vector3 directional_light_position_;
	CascadeData cascadeData;
};