#pragma once
#include "Renderer.h"
using namespace DirectX;

class BulletComponent : public Renderer
{
public:
	BulletComponent();

	// Запускает шарик из точки origin в направлении vel, исчезает через maxRange единиц
	void Spawn(const DirectX::SimpleMath::Vector3& origin,
		const DirectX::SimpleMath::Vector3& vel,
		float maxRange);

	// Текущий вектор скорости (единиц/с)
	DirectX::SimpleMath::Vector3 velocity;

	void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Start()                                                  override;
	void Update(float deltaTime)                                  override;
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;

	ID3D11ShaderResourceView* texture = nullptr;

	// Публичный флаг — активен ли шарик (используется снаружи для проверки попадания)
	bool active = false;

private:
	static constexpr float kInactiveY = -1e7f; // куда прячем неактивный шарик

	ID3D11Buffer* vb = nullptr;
	ID3D11Buffer* ib = nullptr;

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texCoord;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 bitangent;
	};

	DirectX::SimpleMath::Vector3 spawnPos_;
	float maxRange_ = 100.0f;

	BoundingSphere collider;
	Vertex         points[420];
	int            indeces[2400];
};