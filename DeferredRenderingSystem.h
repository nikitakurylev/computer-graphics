#pragma once
#include "RenderingSystem.h"
const int BUFFER_COUNT = 4;

class DeferredRenderingSystem : public RenderingSystem
{
public:
	DeferredRenderingSystem(CubeComponent* cubes);
	virtual void Draw(DisplayWin32* display, std::vector<GameComponent*> Components, Matrix view_matrix, Matrix projection_matrix, LightsParams dynamicLights[10], CascadeData* cascadeData, Vector3 cam_world) override;
	virtual void Initialize(DisplayWin32* Display) override;
private:
	void SetDefferedSetup(int textureWidth, int textureHeight);

	ID3D11RenderTargetView* renderTargetViewArray[BUFFER_COUNT];
	ID3D11ShaderResourceView* shaderResourceViewArray[BUFFER_COUNT];
	ID3D11Texture2D* depthStencilBuffer;
	ID3D11DepthStencilState* depthState;

	ID3D11RasterizerState* rasterizer;
	ID3D11BlendState* blendState;
	ID3D11DepthStencilView* depthStencilView;

	ID3D11VertexShader* dirVertexShader;
	ID3D11PixelShader* dirPixelShader;
	ID3D11PixelShader* pointPixelShader;
};

