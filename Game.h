#pragma once
#include <vector>
#include "DisplayWin32.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <chrono>

class GameComponent;

class Game
{
public:
	Game();
	Game(DisplayWin32 display);
	std::vector<GameComponent*> Components;
	DisplayWin32 Display;
	Microsoft::WRL::ComPtr<ID3D11Device> Device;
	ID3D11DeviceContext* Context;
	MSG msg;
	void Run();
	void Update();
	void Draw();
private:
	void Initialize();
	IDXGISwapChain* SwapChain;
	ID3D11RenderTargetView* RenderView;
	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float TotalTime;
};

