// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "Game.h"
#include "ModelComponent.h"
#include "KatamariComponent.h"


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


	auto sun = KatamariComponent(&game, ball);
	auto floor = ModelComponent(&game, ground);
	floor.immovable = true;
	//sun.scale = Vector3(0.5f, 0.5f, 0.5f);
	
	game.Components.push_back(&sun);
	game.Components.push_back(&floor);
	int count = 20;
	int width = sqrt(count * 20);
	for (int i = 0; i < count; i++) {
		auto sun1 = new ModelComponent(&game, models[rand() % 3]);
		sun1->position = Vector3(rand() % width - (width / 2), 0, rand() % width - (width / 2));
		game.Components.push_back(sun1);
	}

	//game.Components.push_back(&sky);
	game.Initialize();
	chair->Load(game.Display->hWnd, game.Device, game.Context, "chair01.obj");
	ball->Load(game.Display->hWnd, game.Device, game.Context, "soccer_ball.obj");
	ground->Load(game.Display->hWnd, game.Device, game.Context, "ground.obj");
	steve->Load(game.Display->hWnd, game.Device, game.Context, "steve.obj");
	tree->Load(game.Display->hWnd, game.Device, game.Context, "tree.obj");
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
