#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include "GameComponent.h"

class TriangleComponent : public GameComponent
{
public:
	TriangleComponent(Game game);
	void Draw() override;
private:
	DirectX::XMFLOAT4 points[8];
	ID3D11InputLayout* layout;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* vertexShaderByteCode;
	ID3D11PixelShader* pixelShader;
	ID3DBlob* pixelShaderByteCode;
	ID3D11RasterizerState* rastState;
	void Initialize() override;
};

