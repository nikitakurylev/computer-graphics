#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include "GameComponent.h"
#include "OffsetColor.h"

class TriangleComponent : public GameComponent
{
public:
	TriangleComponent(Game* game);
	void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Update(float deltaTime) override;
	void SetColors(float r1, float g1, float b1, float r2, float g2, float b2, float r3, float g3, float b3);
	void SetPositions(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);
private:
	DirectX::XMFLOAT4 points[6];
	ID3D11InputLayout* layout;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* vertexShaderByteCode;
	ID3D11PixelShader* pixelShader;
	ID3DBlob* pixelShaderByteCode;
	ID3D11RasterizerState* rastState;
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
	ID3D11Buffer* constantBuffer;
	OffsetColor offsetColor;
};

