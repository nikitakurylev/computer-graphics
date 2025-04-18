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
	Matrix World;
	Matrix View;
	Vector4 ViewPosition;
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
	Vector3 cam_world;
	void Initialize();
	void Run();
	void Update(float deltaTime);
	void Draw();
	Matrix GetCameraMatrix();
	LightsParams dynamicLights[10];
private:
	void Render(GameComponent* gameComponent, Matrix view, Matrix projection, ID3D11VertexShader* vertex, ID3D11PixelShader* pixel);
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
	ID3D11Buffer* lightTransformBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* dynamicLightBuffer;
	ID3D11SamplerState* TexSamplerState = nullptr;
	ID3D11SamplerState* DepthSamplerState = nullptr;
	Matrix view_matrix;
	Matrix projection_matrix;
	Vector3 cam_rot;
	ID3D11DepthStencilView* depth_stencil_view_ = nullptr;
	ID3D11Texture2D* depth_stencil_buffer_ = nullptr;
	bool fps;
	bool ortho;
	float distance;
	LightsParams light;

	D3D11_VIEWPORT viewport;
	D3D11_VIEWPORT viewport_depth_directional_light_{};
	ID3D11RenderTargetView* render_target_view_depth_directional_light_ = nullptr;
	ID3D11ShaderResourceView* resource_view_depth_directional_light_ = nullptr;

	Vector3 directional_light_position_; 
	Matrix directional_light_view_;
	Matrix directional_light_projection_;

	ID3D11VertexShader* depthVertexShader;
	ID3D11PixelShader* depthPixelShader;
	ID3D11InputLayout* depthInputLayout;
};

