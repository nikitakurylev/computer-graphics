#include "GameComponent.h"
#include "Game.h"

GameComponent::GameComponent(Game* game) : game(game)
{
    scale = Vector3(1, 1, 1);
}

void GameComponent::UpdateWorldMatrix()
{
    world_matrix =
        Matrix::CreateScale(scale)
        * Matrix::CreateFromYawPitchRoll(rotation)
        * Matrix::CreateTranslation(position)
        * (parent == nullptr ? Matrix::Identity : parent->world_matrix);

}

void GameComponent::Initialize(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader)
{
}

void GameComponent::Reload()
{
}

void GameComponent::Update()
{

}

void GameComponent::DestroyResources()
{
}

void GameComponent::Draw()
{
}
