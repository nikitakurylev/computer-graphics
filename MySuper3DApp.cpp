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

	auto sun = SphereComponent(&game);
	auto planet1 = CubeComponent(&game);
	auto planet2 = SphereComponent(&game);
	auto planet3 = CubeComponent(&game);
	auto planet4 = SphereComponent(&game);

	
	sun.scale *= 7.0f;
	sun.mass *= 100000.0f;

	planet1.SetPosition(50, 0, 0);
	planet1.velocity = Vector3(0, 0, 0.1f);
	planet2.SetPosition(0, 0, 50);
	planet2.velocity = Vector3(0.1f, 0, 0);
	planet3.SetPosition(0, 0, -50);
	planet3.velocity = Vector3(-0.1f, 0, 0);
	planet4.SetPosition(-50, 0, 0);
	planet4.velocity = Vector3(0, 0, -0.1f);

	game.Components.push_back(&sun);
	game.Components.push_back(&planet1);
	game.Components.push_back(&planet2);
	game.Components.push_back(&planet3);
	game.Components.push_back(&planet4);

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
