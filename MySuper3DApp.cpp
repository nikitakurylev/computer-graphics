// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


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
		auto sun1 = new SphereComponent(&game, &game.dynamicLights[i]);
		sun1->color = Vector4(1, 1, 0, 1);
		game.Components.push_back(sun1);
		bullets[i] = sun1;
	}

	auto sun = KatamariComponent(&game, ball, bullets);
	auto floor = ModelComponent(&game, ground);
	floor.immovable = true;
	//sun.scale = Vector3(0.5f, 0.5f, 0.5f);
	
	game.Components.push_back(&sun);
	game.Components.push_back(&floor);
	int count = 10;
	int width = sqrt(count * 20);
	for (int i = 0; i < count; i++) {
		auto sun1 = new ModelComponent(&game, models[rand() % 2]);
		sun1->position = Vector3(rand() % width - (width / 2), 0, rand() % width - (width / 2));
		game.Components.push_back(sun1);
	}

	auto huge = new ModelComponent(&game, chair);
	huge->scale *= 50;
	huge->position = Vector3(-50, 0, 51);
	game.Components.push_back(huge);

	auto particles = new ParticleSystemComponent(&game);
	particles->position = Vector3(-40, 0, 40);
	game.Components.push_back(particles);
	auto particles1 = new ParticleSystemComponent(&game);
	particles1->position = Vector3(-30, 0, 40);
	game.Components.push_back(particles1);

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

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
