#pragma once
#include "GameComponent.h"
#include "OffsetColor.h"
class CubeComponent : public GameComponent
{
public:
	CubeComponent(Game* game);
	void Draw() override;
	void Initialize(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader) override;
	void Translate(float x, float y, float z);
	void SetColors(int index, float r, float g, float b);
	void SetSize(float x, float y, float z);
	void SetRotation(float rotation);
	OffsetColor offsetColor;
private:
	struct Vertex
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texCoord;
	};

	Vertex points[8];
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
};

