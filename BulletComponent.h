#pragma once
#include "Renderer.h"

class SkeletalModelComponent;

class BulletComponent : public Renderer
{
public:
    BulletComponent();

    void Spawn(const DirectX::SimpleMath::Vector3& origin,
        const DirectX::SimpleMath::Vector3& vel,
        float maxRange);

    void SetDeformTarget(SkeletalModelComponent* target) { deformTarget_ = target; }

    DirectX::SimpleMath::Vector3 velocity;

    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
    void Start()                                                   override;
    void Update(float deltaTime)                                    override;
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;

    ID3D11ShaderResourceView* texture = nullptr;
    bool active = false;

    float dentRadius = 0.3f;   // радиус вмятины (world units)
    float dentDepth = 0.2f;   // глубина вмятины (world units)

private:
    static constexpr float kInactiveY = -1e7f;

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
    DirectX::SimpleMath::Vector3 shotDir_;
    float                        maxRange_ = 100.0f;

    BoundingSphere          collider;
    Vertex                  points[420];
    int                     indeces[2400];

    SkeletalModelComponent* deformTarget_ = nullptr;
    bool CheckAndApplyDent();
};