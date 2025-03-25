#pragma once
#include <vector>
#include "DisplayWin32.h"
#include "InputDevice.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <chrono>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;
class GameComponent;

struct ConstantBuffer
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProjection;
};

class Game
{
public:
	Game(DisplayWin32* display, InputDevice* input);
	std::vector<GameComponent*> Components;
	DisplayWin32* Display;
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	InputDevice* Input;
	Vector3 cam_pos;
	void Run();
	void Update(float deltaTime);
	void Draw();
	Matrix GetCameraMatrix();
private:
	void Initialize();
	HRESULT	CompileShaderFromFile(LPCWSTR pFileName, const D3D_SHADER_MACRO* pDefines, LPCSTR pEntryPoint, LPCSTR pShaderModel, ID3DBlob** ppBytecodeBlob);
	IDXGISwapChain* SwapChain;
	ID3D11RenderTargetView* RenderView;
	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float TotalTime;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* vertexShaderByteCode;
	ID3D11PixelShader* pixelShader;
	ID3DBlob* pixelShaderByteCode;
	ID3D11InputLayout* layout;
	ID3D11RasterizerState* rastState;
	ID3D11Buffer* constantBuffer;
	Matrix view_matrix;
	Matrix projection_matrix;
	Vector3 cam_rot;
	ID3D11DepthStencilView* depth_stencil_view_ = nullptr;
	ID3D11Texture2D* depth_stencil_buffer_ = nullptr;
	bool fps;
	bool ortho;
	float distance;
};

