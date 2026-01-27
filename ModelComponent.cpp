#include "ModelComponent.h"
#include <directxmath.h>
#include <iostream>
#include "Game.h"
#include "GameObject.h"
#include "Transform.h"

#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif

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

bool ModelComponent::GetGlobalAABB(BoundingBox& outBox)
{
	if (!_meshes || _meshes->empty())
		return false;

	BoundingBox localAABB = _meshes->at(0).aabb;
	for (size_t i = 1; i < _meshes->size(); ++i) {
		BoundingBox::CreateMerged(localAABB, localAABB, _meshes->at(i).aabb);
	}

	if (gameObject) {
		Matrix world = gameObject->GetTransform()->GetMatrix();
		localAABB.Transform(outBox, world);
		return true;
	}
	return false;
}
