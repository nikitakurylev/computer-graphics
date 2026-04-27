#include "BulletComponent.h"
#include "SkeletalMesh/SkeletalModelComponent.h"
#include <directxmath.h>
#include "Component.h"
#include "Game.h"
#include "GameObject.h"
#include "Transform.h"

using namespace DirectX::SimpleMath;

BulletComponent::BulletComponent() {}

void BulletComponent::Spawn(const Vector3& origin, const Vector3& vel, float maxRange)
{
    gameObject->receive_transform_from_backend = false;
    gameObject->send_transform_to_backend = false;

    gameObject->GetTransform()->position = origin;
    spawnPos_ = origin;
    velocity = vel;
    maxRange_ = maxRange;
    active = true;

    shotDir_ = vel;
    float len = shotDir_.Length();
    if (len > 1e-5f) shotDir_ /= len;
}

void BulletComponent::Start()
{
    gameObject->receive_transform_from_backend = false;
    gameObject->send_transform_to_backend = false;
    gameObject->GetTransform()->position.y = kInactiveY;
    active = false;
}

bool BulletComponent::CheckAndApplyDent()
{
    if (!deformTarget_) return false;

    BoundingBox targetAABB;
    if (!deformTarget_->GetGlobalAABB(targetAABB)) return false;
    if (!collider.Intersects(targetAABB)) return false;

    deformTarget_->ApplyDent(collider.Center, shotDir_, dentRadius, dentDepth);
    return true;
}

void BulletComponent::Update(float deltaTime)
{
    if (!active) return;

    auto* tr = gameObject->GetTransform();
    tr->position += velocity * deltaTime;
    collider.Center = tr->position;

    if (CheckAndApplyDent())
    {
        active = false;
        tr->position.y = kInactiveY;
        velocity = Vector3::Zero;
        return;
    }

    if (Vector3::Distance(tr->position, spawnPos_) >= maxRange_)
    {
        active = false;
        tr->position.y = kInactiveY;
        velocity = Vector3::Zero;
    }

    Component::Update(deltaTime);
}

void BulletComponent::Draw(ID3D11Device* /*device*/, ID3D11DeviceContext* context)
{
    if (!active) return;
    UINT strides[] = { sizeof(Vertex) };
    UINT offsets[] = { 0 };
    context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
    context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    context->PSSetShaderResources(0, 1, &texture);
    context->DrawIndexed(2400, 0, 0);
}

void BulletComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* /*context*/)
{
    constexpr float thau = 6.28318530718f;
    for (int j = 0; j <= 20; j++)
    {
        float height = (float)j / 20.0f;
        float t = sqrtf(height * (1.0f - height));
        for (int i = 0; i < 20; i++)
        {
            float s, c;
            DirectX::XMScalarSinCos(&s, &c, thau * (float)i / 20.0f);
            auto pos = DirectX::XMFLOAT3(c * t, height - 0.5f, s * t);
            points[j * 20 + i].position = pos;
            auto n = Vector3(pos.x, pos.y, pos.z);
            n.Normalize();
            points[j * 20 + i].normal = -n;
            points[j * 20 + i].texCoord = { 0, 0 };
            points[j * 20 + i].tangent = { 0, 0, 0 };
            points[j * 20 + i].bitangent = { 0, 0, 0 };
        }
    }
    for (int j = 0; j < 20; j++)
        for (int i = 0; i < 20; i++)
        {
            int idx = j * 20 + i;
            indeces[6 * idx + 0] = idx;
            indeces[6 * idx + 1] = idx + 20 - 1;
            indeces[6 * idx + 2] = idx + 20;
            indeces[6 * idx + 3] = idx + 20;
            indeces[6 * idx + 4] = idx + 1;
            indeces[6 * idx + 5] = idx;
        }

    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT; vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.ByteWidth = sizeof(Vertex) * (UINT)std::size(points);
    D3D11_SUBRESOURCE_DATA vd = {}; vd.pSysMem = points;
    device->CreateBuffer(&vbd, &vd, &vb);

    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT; ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.ByteWidth = sizeof(int) * (UINT)std::size(indeces);
    D3D11_SUBRESOURCE_DATA id = {}; id.pSysMem = indeces;
    device->CreateBuffer(&ibd, &id, &ib);

    collider.Radius = 0.15f;
}