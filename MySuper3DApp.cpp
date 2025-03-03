// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "Game.h"
#include "TriangleComponent.h"


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


int main()
{
	auto game = Game();
	TriangleComponent triangle1 = TriangleComponent(&game);

	triangle1.SetPositions(
		0.6f, 0.5f, 0.0f,
		-0.4f, -0.5f, 0.5f,
		0.6f, -0.5f, 0.5f
	);

	triangle1.SetColors(
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f
	);

	TriangleComponent triangle2 = TriangleComponent(&game);

	triangle2.SetPositions(
		0.4f, 0.5f, 0.0f,
		-0.6f, -0.5f, 0.5f,
		-0.6f, 0.5f, 0.5f
	);

	triangle2.SetColors(
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f
	);

	game.Components.push_back(&triangle1);
	game.Components.push_back(&triangle2);
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
