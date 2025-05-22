#include "Game.h"
#include <d3d11.h>
#include <iostream>
#include "GameComponent.h"
#include "CubeComponent.h"
#include <d3dcompiler.h>
#include "SimpleMath.h"
#include "RenderingSystem.h"
#include "DeferredRenderingSystem.h"

using namespace DirectX::SimpleMath;

Game::Game(DisplayWin32* display, InputDevice* input) : Display(display), Input(input)
{
	auto cubes = new CubeComponent[4]{
	CubeComponent(this),
	CubeComponent(this),
	CubeComponent(this),
	CubeComponent(this)
	};
	cubes[0].SetColor(1, 0, 1);
	cubes[1].SetColor(1, 1, 0);
	cubes[2].SetColor(0, 1, 1);
	cubes[3].SetColor(1, 0, 0);

	Render = new DeferredRenderingSystem(cubes);
}

void Game::Run()
{
	unsigned int frameCount = 0;


	MSG msg = {};
	bool isExitRequested = false;
	auto t = 0.0f;
	while (!isExitRequested) {
		// Handle the windows messages.
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT) {
			isExitRequested = true;
		}

		auto	curTime = std::chrono::steady_clock::now();
		float	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
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

		if (Input->IsKeyDown(Keys::D1))
			fps = true;
		else if (Input->IsKeyDown(Keys::D2))
			fps = false;

		if (Input->IsKeyDown(Keys::D3)) {
			ortho = false;
			projection_matrix = Matrix::CreatePerspectiveFieldOfView(
				DirectX::XM_PIDIV2, Display->ClientWidth / (FLOAT)Display->ClientHeight,
				0.01f, 1000);
		}
		else if (Input->IsKeyDown(Keys::D4)) {
			ortho = true;
			projection_matrix = Matrix::CreateOrthographic(Display->ClientWidth * distance * 0.001f, Display->ClientHeight * distance * 0.001f, 0.01f, 1000);
		}

		if (Input->IsKeyDown(Keys::D5)) {
			cascadeData.debug.x = 1;
		}
		else if (Input->IsKeyDown(Keys::D6)) {
			cascadeData.debug.x = 0;
		}

		cam_rot.y += Input->MouseOffset.x * 0.01f;
		cam_rot.x += Input->MouseOffset.y * 0.01f;
		Input->MouseOffset = Vector2();

		if (fps) {
			auto camera_matrix =
				Matrix::CreateFromYawPitchRoll(cam_rot);
			//* Matrix::CreateTranslation(cam_pos);

			if (Input->IsKeyDown(Keys::W))
				cam_pos += camera_matrix.Forward() * 0.01f;
			if (Input->IsKeyDown(Keys::S))
				cam_pos -= camera_matrix.Forward() * 0.01f;
			if (Input->IsKeyDown(Keys::D))
				cam_pos += camera_matrix.Right() * 0.01f;
			if (Input->IsKeyDown(Keys::A))
				cam_pos -= camera_matrix.Right() * 0.01f;
			if (Input->IsKeyDown(Keys::Space))
				cam_pos.y += 0.01f;
			if (Input->IsKeyDown(Keys::LeftShift))
				cam_pos.y -= 0.01f;
			const auto rotation = Matrix::CreateFromYawPitchRoll(cam_rot);
			const auto target = Vector3::Transform(Vector3::Forward, rotation) + Vector3(cam_pos);
			const auto up_direction = Vector3::Transform(Vector3::Up, rotation);
			view_matrix = Matrix::CreateLookAt(Vector3(cam_pos), target, up_direction);
		}
		else {
			distance = max(1.0f, distance - Input->MouseWheelDelta * 0.01f);
			if (ortho && Input->MouseWheelDelta != 0)
				projection_matrix = Matrix::CreateOrthographic(Display->ClientWidth * distance * 0.001f, Display->ClientHeight * distance * 0.001f, 0.01f, 1000);
			Input->MouseWheelDelta = 0;
			auto lookAtPoint = cam_pos;
			cam_world = Vector3(distance, 0, 0); // distance - расстояние от камеры
			// до точки просмотра
			Matrix rotMat = Matrix::CreateFromYawPitchRoll(Vector3(0, -cam_rot.y, cam_rot.x));
			cam_world = Vector3::Transform(cam_world, rotMat) + lookAtPoint; // Финальная позиция камеры
			view_matrix = Matrix::CreateLookAt(cam_world, lookAtPoint, Vector3::Transform(Vector3::Up, rotMat));
		}

		cascadeData.view_pos = Vector4(cam_world);

		Update(deltaTime);
		Render->Draw(Display, Components, view_matrix, projection_matrix, dynamicLights, &cascadeData, cam_world);
	}

	std::cout << "Hello World!\n";
}

void Game::Update(float deltaTime)
{
	for (GameComponent* gameComponent : Components)
	{
		gameComponent->Update(deltaTime);
	}
}

Matrix Game::GetCameraMatrix()
{
	return Matrix::CreateFromYawPitchRoll(Vector3(0, -cam_rot.y, cam_rot.x));
}

void Game::Initialize()
{

	PrevTime = std::chrono::steady_clock::now();
	TotalTime = 0;

	projection_matrix = Matrix::CreatePerspectiveFieldOfView(
		DirectX::XM_PIDIV2, Display->ClientWidth / (FLOAT)Display->ClientHeight,
		0.01f, 1000);

	directional_light_position_ = Vector3(20, 100, 20);

	auto dir = Vector3(-directional_light_position_.x, -directional_light_position_.y, -directional_light_position_.z);

	dir.Normalize();
	const auto directional_light_direction = Vector4(dir.x, dir.y, dir.z, 1);

	cascadeData.color = Vector4(0.054f, 0.149f, 0.49f, 0);
	cascadeData.direction = directional_light_direction;
	cascadeData.k = Vector4(0.1f, 100.0f, 1.2f, 0);

	for (int i = 0; i < 10; i++) {
		dynamicLights[i].direction = Vector4(i, 3, 0, 0);
		dynamicLights[i].color = Vector4(i % 2, 1, 1 - i % 2, 0);
		dynamicLights[i].k = Vector4(0.1f, 100.0f, 1.2f, 0);
	}

	Render->Initialize(Display);

	for (GameComponent* gameComponent : Components)
	{
		gameComponent->Initialize(Render->Device, Render->Context);
	}
}
