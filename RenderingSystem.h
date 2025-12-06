#pragma once
#include <vector>
#include "DisplayWin32.h"
#include "InputDevice.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <chrono>
#include "SimpleMath.h"
#include "CascadeData.h"

class GameObject;
class CubeComponent;

class RenderingSystem
{
public:
	RenderingSystem(CubeComponent* cubes);
	virtual void Draw(DisplayWin32* display, std::vector<GameObject*> Components, Matrix view_matrix, Matrix projection_matrix, LightsParams dynamicLights[10], CascadeData* cascadeData, Vector3 cam_world);
	virtual void Initialize(DisplayWin32* Display, std::vector<GameObject*> GameObjects);
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
protected:
	void RenderDepthMaps(CascadeData* cascadeData, std::vector<GameObject*> Components, const Matrix& view_matrix, Vector3 cam_world);
	void SetShadowMaps(int offset);
	void UpdateCascadeBuffer(CascadeData* cascadeData);
	void SetColorSampler();
	LPCWSTR vertexShaderName = L"./SimpleTexturedDirectx11/VertexShader.hlsl";
	LPCWSTR pixelShaderName = L"./SimpleTexturedDirectx11/PixelShader.hlsl";
	void Render(GameObject* gameComponent, Matrix view, Matrix projection, ID3D11VertexShader* vertex, ID3D11PixelShader* pixel, Vector3 cam_world);
	void UpdateTransformBuffer(Matrix world_matrix, Matrix view, Matrix projection, Vector3 cam_world);
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	IDXGISwapChain* SwapChain;
	ID3D11RenderTargetView* RenderView;
	ID3D11RasterizerState* rastState;
	ID3D11InputLayout* layout;
	D3D11_VIEWPORT viewport;
	HRESULT CompileShaderFromFile(LPCWSTR pFileName, const D3D_SHADER_MACRO* pDefines, LPCSTR pEntryPoint, LPCSTR pShaderModel, ID3DBlob** ppBytecodeBlob);
private:
	void InitDepthMap(int index, float resolution, DisplayWin32* Display);
	void RenderDepthMap(int index, CascadeData* cascadeData, std::vector<GameObject*> Components, const Matrix& view_matrix, Vector3 cam_world);
	std::vector<Vector4> GetFrustrumCornersWorldSpace(const Matrix& proj, const Matrix& view_matrix);
	Matrix GetCascadeView(const std::vector<Vector4>& corners, int index, Vector4 direction);
	Matrix GetCascadeProjection(const Matrix& lightView, const std::vector<Vector4>& corners, int index);
	ID3D11VertexShader* debugVertexShader;
	ID3D11PixelShader* debugPixelShader;
	CubeComponent* debug_cube;
	Matrix directional_light_projection[4];
	D3D11_VIEWPORT viewport_depth_directional_light_[4];
	ID3D11RenderTargetView* render_target_view_depth_directional_light[4];
	ID3D11ShaderResourceView* resource_view_depth_directional_light[4];
	ID3D11VertexShader* depthVertexShader;
	ID3D11PixelShader* depthPixelShader;
	ID3D11InputLayout* depthInputLayout;
	ID3D11DepthStencilView* depth_stencil_view[4];
	ID3D11Texture2D* depth_stencil_buffer[4];
	ID3D11SamplerState* TexSamplerState = nullptr;
	ID3D11SamplerState* DepthSamplerState = nullptr;
	ID3D11Buffer* constantBuffer;
	ID3D11Buffer* lightTransformBuffer;
	ID3D11Buffer* dynamicLightBuffer;
};

