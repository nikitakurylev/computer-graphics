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
	std::vector<GameObject*> GameObjects;
	DisplayWin32* Display;
	InputDevice* Input;
	RenderingSystem* Render;
	PhysicsSystem Physics;
	AudioSystem Audio;
	Vector3 cam_pos;
	Vector3 cam_world;
	void Initialize();
	void Run();
	void Update(float deltaTime);
	void RenderDebugUI();
	Matrix GetCameraMatrix();
	Matrix view_matrix;
	Matrix projection_matrix;
	ScriptingEngine* scripting_engine;
private:
	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float TotalTime;

	Matrix light_view_proj[4];
	Vector3 cam_rot;

	bool fps;
	bool ortho;
	float distance;

	Vector3 directional_light_position_;
	CascadeData cascadeData;
};

