#pragma once
#include "SimpleMath.h"

using namespace DirectX;

#pragma pack(push, 4)
struct MarshalledVector3
{
public:
	float x;
	float y;
	float z;

	SimpleMath::Vector3 ConvertToVector()
	{
		return SimpleMath::Vector3(x, y, z);
	}
};
#pragma pack(pop)