// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "Game.h"
#include "TriangleComponent.h"
#include "QuadComponent.h"
#include "BoxComponent.h"
#include "BallComponent.h"
#include "PaddleComponent.h"
#include "PlayerPaddleComponent.h"
#include "EnemyPaddleComponent.h"


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


int main()
{
	auto window = DisplayWin32::Instance();
	auto inputDevice = InputDevice();
	auto game = Game(&window, &inputDevice);

	BoxComponent wall1 = BoxComponent(&game);
	wall1.SetPosition(0, 0.55f, 0);
	wall1.SetSize(2, 0.05f);
	BoxComponent wall2 = BoxComponent(&game);
	wall2.SetPosition(0, -0.55f, 0);
	wall2.SetSize(2, 0.05f);

	PlayerPaddleComponent paddle = PlayerPaddleComponent(&game);

	paddle.SetPosition(-0.9, 0, 0);

	BallComponent ball = BallComponent(&game);

	ball.SetSize(0.05, 0.05);

	ball.Velocity.x = 0.01f;
	//ball.Velocity.y = 0.01f;
	
	EnemyPaddleComponent enemy = EnemyPaddleComponent(&game, &ball);
	enemy.SetPosition(0.9f, 0, 0);


	game.Components.push_back(&wall1);
	game.Components.push_back(&wall2);
	game.Components.push_back(&paddle);
	game.Components.push_back(&ball);
	game.Components.push_back(&enemy);
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
