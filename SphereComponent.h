#pragma once
#include "GameComponent.h"
#include "LightsParams.h"
using namespace DirectX;

class SphereComponent : public GameComponent
{
public:
	SphereComponent(Game* game, LightsParams* light);
	Vector3 velocity;
	void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Update(float deltaTime) override;
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
	ID3D11ShaderResourceView* texture;
private:
	ID3D11Buffer* vb;
	ID3D11Buffer* ib;
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texCoord;
	};

	BoundingSphere collider;
	Vertex points[420];
	int indeces[2400];
	LightsParams* light;
};

