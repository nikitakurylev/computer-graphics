#include "GameComponent.h"
#include "Game.h"

GameComponent::GameComponent(Game* game) : game(game)
{
    scale = Vector3(1, 1, 1);
    speed = 0.01f + rand() % 101 / 400.0f;
    auto r = rand() % 1001 / 1000.0f;
    auto g = rand() % 1001 / 1000.0f * (1 - r);
    color = Vector4(r, g, 1.0f - r - g, 1.0f);
    scale = Vector3(1, 1, 1) * (0.1f + rand() % 101 / 200.0f);
    position = Vector3(1.3f + rand() % 101 / 100.0f, 0, 0);
}

void GameComponent::UpdateWorldMatrix()
{
    world_matrix =
        Matrix::CreateScale(scale)
        * Matrix::CreateFromQuaternion(rotation)
        * Matrix::CreateTranslation(position)
        * (parent == nullptr ? Matrix::Identity : parent->world_matrix);
}

void GameComponent::SetPosition(float x, float y, float z)
{
    position = Vector3(x, y, z);
    UpdateWorldMatrix();
}

void GameComponent::Initialize(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader)
{
}

void GameComponent::Reload()
{
}

void GameComponent::Update(float deltaTime)
{
}

void GameComponent::DestroyResources()
{
}

void GameComponent::Draw()
{
}
