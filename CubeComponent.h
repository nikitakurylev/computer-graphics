#pragma once
#include "GameComponent.h"
#include "LightsParams.h"
using namespace DirectX;

class CubeComponent : public GameComponent
{
public:
	CubeComponent(Game* game);
	void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Translate(float x, float y, float z);
	void SetColor(float r, float g, float b);
	void SetSize(float x, float y, float z);
	void SetRotation(float rotation);
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texCoord;
	};

	Vertex points[8];
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
};

