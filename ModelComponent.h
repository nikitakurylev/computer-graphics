#pragma once
#include "GameComponent.h"
#include "SimpleTexturedDirectx11/ModelLoader.h"
class ModelComponent : public GameComponent
{
public:
	ModelComponent(Game* game, std::string fileName);
	void Draw() override;
	void Initialize(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader) override;
	void SetSize(float x, float y, float z);
	void SetRotation(float rotation);
private:
	ModelLoader* ourModel;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	std::string fileName;
};