#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include "GameComponent.h"
#include "OffsetColor.h"

class QuadComponent : public GameComponent
{
public:
	QuadComponent(Game* game);
	void Draw() override;
	void Update() override;
	void SetOffset(float x, float y, float z);
	void SetColors(float r1, float g1, float b1, float r2, float g2, float b2, float r3, float g3, float b3, float r4, float g4, float b4);
	void SetPositions(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
	OffsetColor offsetColor;
private:
	DirectX::XMFLOAT4 points[8];
	ID3D11InputLayout* layout;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* vertexShaderByteCode;
	ID3D11PixelShader* pixelShader;
	ID3DBlob* pixelShaderByteCode;
	ID3D11RasterizerState* rastState;
	void Initialize() override;
	ID3D11Buffer* constantBuffer;
};

