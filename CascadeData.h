#pragma once
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;

struct CascadeData
{
	Matrix ViewProj[4];
	Vector4 view_pos;
	Vector4 position;
	Vector4 color;
	Vector4 k;
	Vector4 debug;
};