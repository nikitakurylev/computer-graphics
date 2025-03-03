#include "TriangleComponent.h"
#include <directxmath.h>
#include <d3dcompiler.h>
#include <iostream>
#include "GameComponent.h"
#include "Game.h"

TriangleComponent::TriangleComponent(Game* game) : GameComponent(game)
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
			MessageBox(game->Display.hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl", Shader_Macros /*macros*/, nullptr /*include*/, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShaderByteCode, &errorPixelCode);

	game->Device->CreateVertexShader(
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		nullptr, &vertexShader);

	game->Device->CreatePixelShader(
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

	game->Device->CreateInputLayout(
		inputElements,
		2,
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		&layout);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game->Device->CreateRasterizerState(&rastDesc, &rastState);

	D3D11_BUFFER_DESC constBufDesc = {};
	constBufDesc.ByteWidth = sizeof(ConstData);
	constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufDesc.MiscFlags = 0;
	constBufDesc.StructureByteStride = 0;

	game->Device->CreateBuffer(&constBufDesc, nullptr, &constantBuffer);
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
	game->Device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

	int indeces[] = { 0,1,2 };
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
	game->Device->CreateBuffer(&indexBufDesc, &indexData, &ib);

	UINT strides[] = { 32 };
	UINT offsets[] = { 0 };
	
	game->Context->RSSetState(rastState);

	game->Context->IASetInputLayout(layout);
	game->Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game->Context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game->Context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game->Context->VSSetShader(vertexShader, nullptr, 0);
	game->Context->PSSetShader(pixelShader, nullptr, 0);
	game->Context->VSSetConstantBuffers(0, 1, &constantBuffer);
	game->Context->DrawIndexed(3, 0, 0);

}

void TriangleComponent::Update()
{
	// If windows signals to end the application then exit out.
	if (game->msg.message == WM_KEYDOWN) {
		auto key = static_cast<unsigned int>(game->msg.wParam);
		switch (key) {
		case 39:
			offsetColor.offset.x += 0.05f;
			break;
		case 37:
			offsetColor.offset.x -= 0.05f;
			break;
		case 40:
			offsetColor.offset.y -= 0.05f;
			break;
		case 38:
			offsetColor.offset.y += 0.05f;
			break;
		case 49:
			offsetColor.color = DirectX::XMFLOAT4(1.0f, 0, 0, 0);
			break;
		case 50:
			offsetColor.color = DirectX::XMFLOAT4(0, 1.0f, 0, 0);
			break;
		case 51:
			offsetColor.color = DirectX::XMFLOAT4(0, 0, 1.0f, 0);
			break;
		case 52:
			offsetColor.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 0);
			break;
		case 53:
			offsetColor.color = DirectX::XMFLOAT4(0, 0, 0, 0);
			break;
		}

		D3D11_MAPPED_SUBRESOURCE res = {};
		game->Context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

		auto dataPtr = reinterpret_cast<float*>(res.pData);
		memcpy(dataPtr, &offsetColor, sizeof(ConstData));
		game->Context->Unmap(constantBuffer, 0);
	}
}

void TriangleComponent::SetColors(float r1, float g1, float b1, float r2, float g2, float b2, float r3, float g3, float b3)
{
	points[1] = DirectX::XMFLOAT4(r1, g1, b1, 1.0f);
	points[3] = DirectX::XMFLOAT4(r2, g2, b2, 1.0f);
	points[5] = DirectX::XMFLOAT4(r3, g3, b3, 1.0f);
}

void TriangleComponent::SetPositions(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
{
	points[0] = DirectX::XMFLOAT4(x1, y1, z1, 1.0f);
	points[2] = DirectX::XMFLOAT4(x2, y2, z2, 1.0f);
	points[4] = DirectX::XMFLOAT4(x3, y3, z3, 1.0f);
}
