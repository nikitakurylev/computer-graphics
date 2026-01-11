#include "ModelComponent.h"
#include <directxmath.h>
#include <iostream>
#include "Game.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

ModelComponent::ModelComponent(std::vector<Mesh>* meshes) : _meshes(meshes)
{
}

void ModelComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{

}

void ModelComponent::Draw(ID3D11Device* device, ID3D11DeviceContext* context) {
	for (size_t i = 0; i < _meshes->size(); ++i) {
		_meshes->at(i).Draw(context);
	}
}
