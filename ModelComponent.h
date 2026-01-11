#pragma once
#include "Renderer.h"
#include "SimpleTexturedDirectx11/Mesh.h"
class ModelComponent : public Renderer
{
public:
	ModelComponent(std::vector<Mesh>* meshes);
	void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
private:
	std::vector<Mesh>* _meshes;
	std::string fileName;
};