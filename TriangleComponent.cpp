#include "TriangleComponent.h"
#include <directxmath.h>
#include <d3dcompiler.h>
#include <iostream>
#include "GameComponent.h"
#include "Game.h"

TriangleComponent::TriangleComponent(Game game) : GameComponent(game)
{
	Initialize();
}

void TriangleComponent::Initialize()
{
	vertexShaderByteCode = nullptr;
	ID3DBlob* errorVertexCode = nullptr;
	auto res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShaderByteCode,
		&errorVertexCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorVertexCode) {
			char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(game.Display.hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl", Shader_Macros /*macros*/, nullptr /*include*/, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShaderByteCode, &errorPixelCode);

	game.Device->CreateVertexShader(
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		nullptr, &vertexShader);

	game.Device->CreatePixelShader(
		pixelShaderByteCode->GetBufferPointer(),
		pixelShaderByteCode->GetBufferSize(),
		nullptr, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	game.Device->CreateInputLayout(
		inputElements,
		2,
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		&layout);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game.Device->CreateRasterizerState(&rastDesc, &rastState);

	points[0] = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	points[1] = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	points[2] = DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f);
	points[3] = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	points[4] = DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f);
	points[5] = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	points[6] = DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f);
	points[7] = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

void TriangleComponent::Draw() {

	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * std::size(points);

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = points;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	ID3D11Buffer* vb;
	game.Device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

	int indeces[] = { 0,1,2, 1,0,3 };
	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * std::size(indeces);

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indeces;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	ID3D11Buffer* ib;
	game.Device->CreateBuffer(&indexBufDesc, &indexData, &ib);

	UINT strides[] = { 32 };
	UINT offsets[] = { 0 };
	
	game.Context->RSSetState(rastState);

	game.Context->IASetInputLayout(layout);
	game.Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game.Context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game.Context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game.Context->VSSetShader(vertexShader, nullptr, 0);
	game.Context->PSSetShader(pixelShader, nullptr, 0);
}
