#pragma once
#include "Renderer.h"
using namespace DirectX;

class BulletComponent : public Renderer
{
public:
	BulletComponent();
	Vector3 velocity;
	void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Start() override;
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
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 bitangent;
	};

	BoundingSphere collider;
	Vertex points[420];
	int indeces[2400];
};

