#include "ModelComponent.h"
#include <directxmath.h>
#include <iostream>
#include "GameComponent.h"
#include "Game.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

ModelComponent::ModelComponent(Game* game, ModelLoader* model) : GameComponent(game), ourModel(model)
{
}

void ModelComponent::Initialize(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader)
{

	VertexShader = vertexShader;
	PixelShader = pixelShader;
}

void ModelComponent::Draw() {

	game->Context->VSSetShader(VertexShader, 0, 0);
	game->Context->PSSetShader(PixelShader, 0, 0);
	ourModel->Draw(game->Context);
}

void ModelComponent::SetRotation(float rot)
{

}

void ModelComponent::SetSize(float x, float y, float z)
{
	scale = Vector3(x, y, z);
	UpdateWorldMatrix();
}
