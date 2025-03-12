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
	auto window = DisplayWin32::Instance();
	auto inputDevice = InputDevice();
	auto game = Game(&window, &inputDevice);

	auto cube = SphereComponent(&game);

	cube.SetColors(0, 1, 0, 0);
	cube.SetColors(1, 1, 0, 0);
	cube.SetColors(2, 1, 0, 0);
	cube.UpdateWorldMatrix();

	auto cube1 = CubeComponent(&game);

	cube1.SetSize(0.05f, 0.05f, 0.05f);
	cube1.SetColors(0, 1, 0, 0);
	cube1.SetColors(1, 1, 0, 0);
	cube1.SetColors(2, 1, 0, 0);
	cube1.position = Vector3(1, 0, 0);
	cube1.parent = &cube;
	cube1.UpdateWorldMatrix();

	auto cube2 = CubeComponent(&game);

	cube2.SetSize(0.02f, 0.02f, 0.02f);
	cube2.SetColors(0, 1, 0, 0);
	cube2.SetColors(1, 1, 0, 0);
	cube2.SetColors(2, 1, 0, 0);
	cube2.position = Vector3(0.2f, 0, 0);
	cube2.parent = &cube1;
	cube2.UpdateWorldMatrix();

	game.Components.push_back(&cube);
	game.Components.push_back(&cube1);
	game.Components.push_back(&cube2);
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
