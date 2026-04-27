#include "Game.h"
#include <d3d11.h>
#include <iostream>
#include <algorithm>
#include "GameObject.h"
#include "CubeComponent.h"
#include "BulletComponent.h"
#include <d3dcompiler.h>
#include "SimpleMath.h"
#include "RenderingSystem.h"
#include "DeferredRenderingSystem.h"
#include "BenchmarkLogger.h"

using namespace DirectX::SimpleMath;

Game::Game(DisplayWin32* display, InputDevice* input, RenderingSystem* render, ScriptingEngine* scriptingEngine)
	: Display(display), Input(input), Render(render), scripting_engine(scriptingEngine), Physics(), Audio() {
}

Game::~Game()
{
	BenchmarkLogger::Instance().Flush("benchmark.csv");
}

// ============================================================
void Game::RegisterProjectile(BulletComponent* bullet)
{
	projectilePool_.push_back(bullet);
}

// ============================================================
void Game::Run()
{
	unsigned int frameCount = 0;

	MSG msg = {};
	bool isExitRequested = false;
	auto t = 0.0f;
	while (!isExitRequested) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			isExitRequested = true;

		auto  curTime = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;
		t += deltaTime;
		TotalTime += deltaTime;
		frameCount++;

		if (TotalTime > 1.0f) {
			float fps = frameCount / TotalTime;
			TotalTime -= 1.0f;

			WCHAR text[256];
			swprintf_s(text, TEXT("FPS: %f"), fps);
			SetWindowText(Display->hWnd, text);

			frameCount = 0;
		}

		// --- Переключение отладочных режимов (D5/D6 оставляем) ---
		if (Input->IsKeyDown(Keys::D5))
			cascadeData.debug.x = 1;
		else if (Input->IsKeyDown(Keys::D6))
			cascadeData.debug.x = 0;

		// --------------------------------------------------------
		//  Свободная летающая FPS-камера (всегда активна)
		// --------------------------------------------------------
		cam_rot.y -= Input->MouseOffset.x * 0.003f;
		cam_rot.x -= Input->MouseOffset.y * 0.003f;
		// Ограничиваем pitch, чтобы не перевернуться
		cam_rot.x = std::clamp(cam_rot.x, -DirectX::XM_PIDIV2 + 0.01f, DirectX::XM_PIDIV2 - 0.01f);
		Input->MouseOffset = Vector2();

		const auto rotation = Matrix::CreateFromYawPitchRoll(cam_rot);
		const auto camForward = Vector3::Transform(Vector3::Forward, rotation);
		const auto camRight = Vector3::Transform(Vector3::Right, rotation);
		const auto camUp = Vector3::Transform(Vector3::Up, rotation);

		constexpr float camSpeed = 10.0f; // единиц/с
		if (Input->IsKeyDown(Keys::W))          cam_pos += camForward * camSpeed * deltaTime;
		if (Input->IsKeyDown(Keys::S))          cam_pos -= camForward * camSpeed * deltaTime;
		if (Input->IsKeyDown(Keys::D))          cam_pos += camRight * camSpeed * deltaTime;
		if (Input->IsKeyDown(Keys::A))          cam_pos -= camRight * camSpeed * deltaTime;
		if (Input->IsKeyDown(Keys::Space))      cam_pos += camUp * camSpeed * deltaTime;
		if (Input->IsKeyDown(Keys::LeftShift))  cam_pos -= camUp * camSpeed * deltaTime;

		const auto target = cam_pos + camForward;
		view_matrix = Matrix::CreateLookAt(cam_pos, target, camUp);
		cam_world = cam_pos; // используется системой теней

		cascadeData.view_pos = Vector4(cam_world);

		// --------------------------------------------------------
		//  Стрельба
		// --------------------------------------------------------
		UpdateShooting(deltaTime);

		Update(deltaTime);
		Render->Draw(Display, GameObjects, view_matrix, projection_matrix, &cascadeData, cam_world);

		RenderDebugUI();
	}
}

// ============================================================
void Game::UpdateShooting(float deltaTime)
{
	if (projectilePool_.empty()) return;

	bool xDown = Input->IsKeyDown(Keys::X);

	// Один выстрел за нажатие — срабатываем только на передний фронт
	if (xDown && !lmbWasDown_)
	{
		// Выбираем следующий шарик из пула по кругу
		BulletComponent* bullet = projectilePool_[nextProjectile_ % (int)projectilePool_.size()];
		nextProjectile_ = (nextProjectile_ + 1) % (int)projectilePool_.size();

		const auto rotation = Matrix::CreateFromYawPitchRoll(cam_rot);
		const auto direction = Vector3::Transform(Vector3::Forward, rotation);

		// Спавним чуть впереди камеры, чтобы не было самопересечений
		bullet->Spawn(cam_pos + direction * 1.5f, direction * kProjectileSpeed, kProjectileRange);
	}

	lmbWasDown_ = xDown;
}

// ============================================================
void Game::RenderDebugUI()
{
	static bool showAABB = false;
	static bool f1WasPressed = false;
#ifdef DEBUG_CULLING
	static int  frameCounter = 0;
#endif

	if (Input->IsKeyDown(Keys::F1)) {
		if (!f1WasPressed) {
			showAABB = !showAABB;
			Render->SetDebugAABBMode(showAABB);
			f1WasPressed = true;
		}
	}
	else {
		f1WasPressed = false;
	}

#ifdef DEBUG_CULLING
	if (++frameCounter >= 60) {
		int rendered, culled, total;
		Render->GetCullingStats(rendered, culled, total);
		float cullingRatio = total > 0 ? (culled * 100.0f) / total : 0.0f;
		std::cout << "[Frustum Culling] Rendered: " << rendered
			<< " | Culled: " << culled
			<< " | Total: " << total
			<< " | Culling: " << cullingRatio << "%" << std::endl;
		frameCounter = 0;
	}
#endif
}

// ============================================================
void Game::Update(float deltaTime)
{
	Physics.Update(deltaTime);
	for (GameObject* gameObject : GameObjects)
		gameObject->Update(deltaTime);
}

// ============================================================
Matrix Game::GetCameraMatrix()
{
	return Matrix::CreateFromYawPitchRoll(cam_rot);
}

// ============================================================
void Game::Initialize()
{
	PrevTime = std::chrono::steady_clock::now();
	TotalTime = 0;
	ortho = false;
	distance = 10.0f;

	cam_pos = Vector3(0.0f, 2.0f, -5.0f); // стартовая позиция камеры — чуть перед Y_Bot

	projection_matrix = Matrix::CreatePerspectiveFieldOfView(
		DirectX::XM_PIDIV2,
		Display->ClientWidth / (FLOAT)Display->ClientHeight,
		0.01f, 1000.0f);

	directional_light_position_ = Vector3(50, 100, 20);
	auto dir = Vector3(-directional_light_position_.x, -directional_light_position_.y, -directional_light_position_.z);
	dir.Normalize();
	const auto directional_light_direction = Vector4(dir.x, dir.y, dir.z, 1);

	cascadeData.color = Vector4(0.054f, 0.149f, 0.49f, 0);
	cascadeData.position = directional_light_direction;
	cascadeData.k = Vector4(0.1f, 100.0f, 1.2f, 0);

	Render->Initialize(GameObjects);

	for (GameObject* gameObject : GameObjects)
		gameObject->Start();
}