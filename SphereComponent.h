#pragma once
#include "GameComponent.h"
class SphereComponent : public GameComponent
{
public:
	SphereComponent(Game* game);
	void Draw() override;
	void Update() override;
	void Initialize(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader) override;
	void SetColors(int index, float r, float g, float b);
	void SetRotation(float rotation);
private:
	struct Vertex
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texCoord;
	};

	Vertex points[420];
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	int indeces[2400];
};

