#pragma once
#include "Component.h"
#include <d3d11.h>
using namespace DirectX;
class Renderer : public Component
{
public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
	virtual void Draw(ID3D11Device* device, ID3D11DeviceContext* context);
};

