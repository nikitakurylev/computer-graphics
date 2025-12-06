#pragma once
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;
class Transform
{
public:
	Transform();
	void Update();
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	Transform* parent = nullptr;
	bool immovable = false;
	const Matrix GetMatrix() const { return world_matrix; };
private:
	Matrix world_matrix;
};

