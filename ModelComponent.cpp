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

void ModelComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{

}

void ModelComponent::Draw(ID3D11Device* device, ID3D11DeviceContext* context) {
	ourModel->Draw(context);
}

void ModelComponent::SetRotation(float rot)
{

}

void ModelComponent::SetSize(float x, float y, float z)
{
	scale = Vector3(x, y, z);
	UpdateWorldMatrix();
}
