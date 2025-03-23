#include "GameComponent.h"
#include "Game.h"
#include <iostream>

GameComponent::GameComponent(Game* game) : game(game)
{
    scale = Vector3(1, 1, 1);
    speed = 0.01f + rand() % 101 / 400.0f;
    auto r = rand() % 1001 / 1000.0f;
    auto g = rand() % 1001 / 1000.0f * (1 - r);
    color = Vector4(r, g, 1.0f - r - g, 1.0f);
    auto size = (0.1f + rand() % 101 / 200.0f);
    scale = Vector3(10, 10, 10) * size;
    position = Vector3(1.3f + rand() % 101 / 100.0f, 0, 0);
    mass = size * 0.0001f;
}

void GameComponent::UpdateWorldMatrix()
{
    world_matrix =
        Matrix::CreateScale(scale)
        * Matrix::CreateFromYawPitchRoll(rotation)
        * Matrix::CreateTranslation(position)
        * Matrix::CreateFromYawPitchRoll(rotation)
        * (parent == nullptr ? Matrix::Identity : Matrix::CreateTranslation(parent->world_matrix.Translation()));

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
    rotation.y += speed * deltaTime;

    for (GameComponent* gameComponent : game->Components)
    {
        if (gameComponent == this)
            continue;
        auto directon = gameComponent->position - position;
        auto R = directon.Length();
        velocity += deltaTime * 9.81f * gameComponent->mass / (R * R * R) * directon;
    }
    
    position += velocity;

    UpdateWorldMatrix();
}

void GameComponent::DestroyResources()
{
}

void GameComponent::Draw()
{
}
