// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "Game.h"
#include "ModelComponent.h"
#include "KatamariComponent.h"
#include "ParticleSystemComponent.h"
#include "PointLightComponent.h"
#include "DeferredRenderingSystem.h"
#include "SimpleTexturedDirectx11/ModelLoader.h"
#include "SimpleTexturedDirectx11/SceneLoader.h"

#include "TestStage1.h"
#include "TestStage2.h"
#include "TestStage3.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


int main()
{
	// “≈—“»–Œ¬¿Õ»≈ ›“¿œŒ¬
	AI::RunStage1Tests();
	AI::RunStage2Tests();
	AI::RunStage3Tests();

	srand(1);

	auto window = DisplayWin32::Instance();
	auto inputDevice = InputDevice();
	auto render = DeferredRenderingSystem(&window);
	auto game = Game(&window, &inputDevice, &render);

	auto modelLoader = ModelLoader(window.hWnd, render.Device, render.Context);
	auto sceneLoader = SceneLoader(&game, window.hWnd, render.Device, render.Context);
	auto ball = modelLoader.Load("soccer_ball.obj");
	auto ground = modelLoader.Load("ground.obj");
	auto chair = modelLoader.Load("chair01.obj");
	auto steve = modelLoader.Load("steve.obj");
	auto tree = modelLoader.Load("tree.obj");

	std::vector<Mesh>* models[3] = {chair, steve, tree};

	BulletComponent* bullets[10];
	for (int i = 0; i < 10; i++) {
		auto bulletGameObject = new GameObject(&game);
		auto bulletComponent = new BulletComponent();
		auto lightComponent = new PointLightComponent(Vector4(1,1,1,1), 20);
		bulletGameObject->AddComponent(bulletComponent);
		bulletGameObject->AddComponent(lightComponent);
		game.GameObjects.push_back(bulletGameObject);
		bullets[i] = bulletComponent;
	}

	auto katamariComponent = KatamariComponent();
	auto katamariModel = ModelComponent(ball);
	auto katamariLight = PointLightComponent(Vector4(1, 1, 1, 1), 20);
	auto katamariGameObject = GameObject(&game);
	katamariGameObject.AddComponent(&katamariModel);
	katamariGameObject.AddComponent(&katamariComponent);
	katamariGameObject.AddComponent(&katamariLight);
	auto floorComponent = ModelComponent(ground);
	auto floorGameObject = GameObject(&game);
	floorGameObject.AddComponent(&floorComponent);
	floorGameObject.GetTransform()->immovable = true;
	
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

	//auto particlesComponent = ParticleSystemComponent();
	//auto particlesGameObject = GameObject(&game);
	//particlesGameObject.AddComponent(&particlesComponent);
	//particlesGameObject.GetTransform()->position = Vector3(-40, 0, 40);
	//game.GameObjects.push_back(&particlesGameObject);

	auto sceneObjects = sceneLoader.Load("untitled.glb");

	for (GameObject* gameObject : *sceneObjects)
	{
		game.GameObjects.push_back(gameObject);
	}

	sceneObjects->at(sceneObjects->size() - 1)->GetTransform()->immovable = true;

	game.Initialize();

	for (int i = 0; i < 10; i++) {
		bullets[i]->texture = ball->at(0).textures_[0].texture;
	}
	game.Run();
}
