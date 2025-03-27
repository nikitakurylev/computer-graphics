#include "GameComponent.h"
#include "Game.h"

GameComponent::GameComponent(Game* game) : game(game)
{
    scale = Vector3(1, 1, 1);
    speed = 2;
}

void GameComponent::UpdateWorldMatrix()
{
    world_matrix =
        Matrix::CreateScale(scale)
        * Matrix::CreateFromQuaternion(rotation)
        * Matrix::CreateTranslation(position)
        * (parent == nullptr ? Matrix::Identity :
            Matrix::CreateFromQuaternion(parent->rotation)
            * Matrix::CreateTranslation(parent->position));
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
    UpdateWorldMatrix();
}

void GameComponent::DestroyResources()
{
}

void GameComponent::Draw()
{
}
