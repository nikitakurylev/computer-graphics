#pragma once
#include "GameComponent.h"
#include "LightsParams.h"
using namespace DirectX;

class SphereComponent : public GameComponent
{
public:
	SphereComponent(Game* game, LightsParams* light);
	Vector3 velocity;
	void Draw() override;
	void Update(float deltaTime) override;
	void Initialize(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader) override;
	ID3D11ShaderResourceView* texture;
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texCoord;
	};

	BoundingSphere collider;
	Vertex points[420];
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	int indeces[2400];
	LightsParams* light;
};

