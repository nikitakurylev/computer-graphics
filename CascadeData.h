#pragma once
#include "SimpleMath.h"

struct CascadeData
{
	Matrix ViewProj[4];
	Vector4 Distances;
	Vector4 direction;
	Vector4 color;
	Vector4 k;
	Vector4 debug;
};