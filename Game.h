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
class CubeComponent;

struct ConstantBuffer
{
	Matrix World;
	Matrix View;
	Vector4 ViewPosition;
};

struct CascadeData
{
	Matrix ViewProj[4];
	Vector4 Distances;
	Vector4 direction;
	Vector4 color;
	Vector4 k;
	Vector4 debug;
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
	void InitDepthMap(int index, float resolution);
	void RenderDepthMap(int index);
	std::vector<Vector4> GetFrustrumCornersWorldSpace(const Matrix& proj);
	Matrix GetCascadeView(const std::vector<Vector4>& corners, int index);
	Matrix GetCascadeProjection(const Matrix& lightView, const std::vector<Vector4>& corners, int index);
	IDXGISwapChain* SwapChain;
	ID3D11RenderTargetView* RenderView;
	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float TotalTime;
	ID3D11VertexShader* vertexShader;
	ID3D11VertexShader* debugVertexShader;
	ID3DBlob* vertexShaderByteCode;
	ID3D11PixelShader* pixelShader;
	ID3D11PixelShader* debugPixelShader;
	ID3DBlob* pixelShaderByteCode;
	ID3D11InputLayout* layout;
	ID3D11RasterizerState* rastState;
	ID3D11Buffer* constantBuffer;
	ID3D11Buffer* lightTransformBuffer;
	ID3D11Buffer* dynamicLightBuffer;
	ID3D11SamplerState* TexSamplerState = nullptr;
	ID3D11SamplerState* DepthSamplerState = nullptr;
	Matrix view_matrix;
	Matrix projection_matrix;
	Matrix light_view_proj[4];
	Vector3 cam_rot;
	ID3D11DepthStencilView* depth_stencil_view[4];
	ID3D11Texture2D* depth_stencil_buffer[4];
	bool fps;
	bool ortho;
	float distance;

	D3D11_VIEWPORT viewport;
	D3D11_VIEWPORT viewport_depth_directional_light_[4];
	ID3D11RenderTargetView* render_target_view_depth_directional_light[4];
	ID3D11ShaderResourceView* resource_view_depth_directional_light[4];
	CubeComponent* debug_cube;

	Vector3 directional_light_position_; 
	Matrix directional_light_projection[4];

	ID3D11VertexShader* depthVertexShader;
	ID3D11PixelShader* depthPixelShader;
	ID3D11InputLayout* depthInputLayout;

	CascadeData cascadeData;
};

