#pragma once
#include <vector>
#include "DisplayWin32.h"
#include "InputDevice.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <chrono>
#include "SimpleMath.h"
#include <DirectXCollision.h>
#include "CascadeData.h"
#include "FrustumCulling.h"

class GameObject;
class CubeComponent;

struct PointLightData {
	Vector4 dyn_position;
	Vector4 dyn_color;
	Vector4 dyn_k; // x=radius, y=shininess, z=specular, w=unused
};

struct MaterialBufferData {
	Vector4 baseColorFactor;
	Vector4 materialParams;
};

class RenderingSystem
{
public:
	RenderingSystem(DisplayWin32* Display, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);
	virtual void Draw(DisplayWin32* display, std::vector<GameObject*> Components, Matrix view_matrix, Matrix projection_matrix, CascadeData* cascadeData, Vector3 cam_world);
	virtual void Initialize(std::vector<GameObject*> GameObjects);
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;

	void GetCullingStats(int& rendered, int& culled, int& total) const {
		rendered = debugRenderedCount;
		culled = debugCulledCount;
		total = debugTotalCount;
	}

	void SetDebugAABBMode(bool enabled) { debugShowAABB = enabled; }
	bool GetDebugAABBMode() const { return debugShowAABB; }
protected:
	void RenderDepthMaps(CascadeData* cascadeData, std::vector<GameObject*> Components, const Matrix& view_matrix, Vector3 cam_world);
	void SetShadowMaps(int offset);
	void UpdateCascadeBuffer(CascadeData* cascadeData);
	void SetColorSampler();
	void InitEnvironmentResources();
	void BindEnvironmentResources(int slot);
	LPCWSTR vertexShaderName = L"./SimpleTexturedDirectx11/VertexShader.hlsl";
	LPCWSTR pixelShaderName = L"./SimpleTexturedDirectx11/PixelShader.hlsl";
	void Render(GameObject* gameComponent, Matrix view, Matrix projection, ID3D11VertexShader* vertex, ID3D11PixelShader* pixel, Vector3 cam_world, bool culling = false, bool drawDebugAABB = true);

	ManualFrustumCuller manualFrustumCuller;
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
	void InitDefaultMaterialResources();
	void BindDefaultMaterialResources();
	ID3D11VertexShader* debugVertexShader;
	ID3D11PixelShader* debugPixelShader;
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
	ID3D11Buffer* defaultMaterialBuffer_ = nullptr;
	ID3D11ShaderResourceView* defaultWhiteTexture_ = nullptr;
	ID3D11ShaderResourceView* defaultNormalTexture_ = nullptr;
	ID3D11ShaderResourceView* environmentMap_ = nullptr;
	PointLightData dynamicLights[10];
protected:
	int debugRenderedCount = 0;
	int debugCulledCount = 0;
	int debugTotalCount = 0;
	bool debugShowAABB = false;

	void RenderDebugAABB(const DirectX::BoundingBox& aabb, const Matrix& view, const Matrix& projection, const Vector3& cam_world, bool isCulled);
	ID3D11Buffer* debugLineVertexBuffer = nullptr;
	ID3D11VertexShader* debugLineVertexShader = nullptr;
	ID3D11PixelShader* debugLinePixelShader = nullptr;
	ID3D11InputLayout* debugLineInputLayout = nullptr;
	void InitDebugLineRenderer();
};
