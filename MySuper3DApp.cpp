// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "Game.h"
#include "CubeComponent.h"
#include "SphereComponent.h"


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

	auto sky = SphereComponent(&game);
	sky.speed = 0;
	sky.color = Vector4(0, 0, 0.01f, 1.0f);
	sky.scale = Vector3(1, 1, 1) * 1000.0f;
	auto sun = SphereComponent(&game);
	auto sunOrbit2 = GameComponent(&game);
	auto sunOrbit3 = GameComponent(&game);

	sun.SetPosition(0, 0, 0);
	sun.scale = Vector3(0.7f, 0.7f, 0.7f);
	sunOrbit2.SetPosition(0, 0, 0);
	sunOrbit2.scale = Vector3(1, 1, 1);
	sunOrbit3.SetPosition(0, 0, 0);
	sunOrbit3.scale = Vector3(1, 1, 1);

	auto planet1 = CubeComponent(&game);
	planet1.parent = &sun;
	planet1.speed = 100.0f;
	planet1.SetPosition(2, 0, 0);
	

	auto planet2 = SphereComponent(&game);
	planet2.parent = &sunOrbit2;
	planet2.SetPosition(4, 0, 0);

	auto planet3 = CubeComponent(&game);
	planet3.parent = &sunOrbit3;
	planet3.SetPosition(6, 0, 0);

	auto planet4 = CubeComponent(&game);
	planet4.parent = &sun;
	planet4.SetPosition(-2, 0, 0);

	auto planet5 = CubeComponent(&game);
	planet5.parent = &sunOrbit2;
	planet5.SetPosition(-4, 0, 0);

	auto moon1 = SphereComponent(&game);
	auto moon2 = CubeComponent(&game);
	auto moon3 = SphereComponent(&game);
	auto moon4 = SphereComponent(&game);

	moon1.parent = &planet1;
	moon2.parent = &planet2;
	moon3.parent = &planet3;
	moon4.parent = &planet5;

	//game.Components.push_back(&sky);
	game.Components.push_back(&sun);
	game.Components.push_back(&sunOrbit2);
	game.Components.push_back(&sunOrbit3);
	game.Components.push_back(&planet1);
	game.Components.push_back(&planet2);
	game.Components.push_back(&planet3);
	game.Components.push_back(&planet4);
	game.Components.push_back(&planet5);
	game.Components.push_back(&moon1);
	game.Components.push_back(&moon2);
	game.Components.push_back(&moon3);
	game.Components.push_back(&moon4);

	for (int i = 0; i < 0; i++) {
		auto sunOrbitX = new GameComponent(&game);
		sunOrbitX->SetPosition(0, 0, 0);
		sunOrbitX->scale = Vector3(1, 1, 1);
		GameComponent* planetX = i % 2 == 0 ? (GameComponent*) new CubeComponent(&game) : (GameComponent*) new SphereComponent(&game);
		planetX->parent = sunOrbitX;
		planetX->SetPosition(2 + i * 0.1f, 0, 0);

		game.Components.push_back(sunOrbitX);
		game.Components.push_back(planetX);
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
