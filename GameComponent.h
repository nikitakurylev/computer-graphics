#pragma once
#include "Game.h"
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;
class GameComponent
{
public:
	GameComponent(Game* game);
	void UpdateWorldMatrix();
	void SetPosition(float x, float y, float z);
	virtual void Draw();
	virtual void Update();
	virtual void Initialize(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader);
	GameComponent* parent = nullptr;
	float speed;

	Vector3 position;
	Vector3 rotation;
	Vector3 scale;
	Vector4 color;
	Matrix world_matrix;
protected:
	Game* game;
private:
	void Reload();
	void DestroyResources();
};

