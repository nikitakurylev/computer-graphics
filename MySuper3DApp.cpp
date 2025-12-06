// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "Game.h"
#include "ModelComponent.h"
#include "KatamariComponent.h"
#include "ParticleSystemComponent.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


int main()
{
	srand(1);

	auto window = DisplayWin32::Instance();
	auto inputDevice = InputDevice();
	auto game = Game(&window, &inputDevice);

	auto ball = new ModelLoader;
	auto ground = new ModelLoader;
	
	auto chair = new ModelLoader;
	auto steve = new ModelLoader;
	auto tree = new ModelLoader;
	

	ModelLoader* models[3] = {chair, steve, tree};

	SphereComponent* bullets[10];
	for (int i = 0; i < 10; i++) {
		auto bulletGameObject = new GameObject(&game);
		auto bulletComponent = new SphereComponent(&game.dynamicLights[i]);
		bulletGameObject->AddComponent(bulletComponent);
		game.GameObjects.push_back(bulletGameObject);
		bullets[i] = bulletComponent;
	}

	auto katamariComponent = KatamariComponent(ball, bullets);
	auto katamariGameObject = GameObject(&game);
	katamariGameObject.AddComponent(&katamariComponent);
	auto floorComponent = ModelComponent(ground);
	auto floorGameObject = GameObject(&game);
	floorGameObject.AddComponent(&floorComponent);
	floorGameObject.GetTransform()->immovable = true;
	//sun.scale = Vector3(0.5f, 0.5f, 0.5f);
	
	game.GameObjects.push_back(&katamariGameObject);
	game.GameObjects.push_back(&floorGameObject);
	int count = 10;
	int width = sqrt(count * 20);
	for (int i = 0; i < count; i++) {
		auto clutterComponent = new ModelComponent(models[rand() % 2]);
		auto clutterGameObject = new GameObject(&game);
		clutterGameObject->AddComponent(clutterComponent);
		clutterGameObject->GetTransform()->position = Vector3(rand() % width - (width / 2), 0, rand() % width - (width / 2));
		game.GameObjects.push_back(clutterGameObject);
	}

	auto hugeComponent = ModelComponent(chair);
	auto hugeGameObject = GameObject(&game);
	hugeGameObject.AddComponent(&hugeComponent);
	hugeGameObject.GetTransform()->scale *= 50;
	hugeGameObject.GetTransform()->position = Vector3(-50, 0, 51);
	game.GameObjects.push_back(&hugeGameObject);

	auto particlesComponent = ParticleSystemComponent();
	auto particlesGameObject = GameObject(&game);
	particlesGameObject.AddComponent(&particlesComponent);
	particlesGameObject.GetTransform()->position = Vector3(-40, 0, 40);
	game.GameObjects.push_back(&particlesGameObject);

	//game.Components.push_back(&sky);
	game.Initialize();
	chair->Load(game.Display->hWnd, game.Render->Device, game.Render->Context, "chair01.obj");
	ball->Load(game.Display->hWnd, game.Render->Device, game.Render->Context, "soccer_ball.obj");
	ground->Load(game.Display->hWnd, game.Render->Device, game.Render->Context, "ground.obj");
	steve->Load(game.Display->hWnd, game.Render->Device, game.Render->Context, "steve.obj");
	tree->Load(game.Display->hWnd, game.Render->Device, game.Render->Context, "tree.obj");
	for (int i = 0; i < 10; i++) {
		bullets[i]->texture = ball->textures_loaded_[0].texture;
	}
	game.Run();
}
