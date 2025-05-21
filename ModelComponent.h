#pragma once
#include "GameComponent.h"
#include "SimpleTexturedDirectx11/ModelLoader.h"
class ModelComponent : public GameComponent
{
public:
	ModelComponent(Game* game, ModelLoader* model);
	void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void SetSize(float x, float y, float z);
	void SetRotation(float rotation);
private:
	ModelLoader* ourModel;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	std::string fileName;
};