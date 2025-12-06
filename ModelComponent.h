#pragma once
#include "Renderer.h"
#include "SimpleTexturedDirectx11/ModelLoader.h"
class ModelComponent : public Renderer
{
public:
	ModelComponent(ModelLoader* model);
	void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
private:
	ModelLoader* ourModel;
	std::string fileName;
};