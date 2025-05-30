#include "QuadComponent.h"
#include <directxmath.h>
#include <d3dcompiler.h>
#include <iostream>
#include "GameComponent.h"
#include "Game.h"

QuadComponent::QuadComponent(Game* game) : GameComponent(game)
{
	ShaderPath = L"./Shaders/MyVeryFirstShader.hlsl";
}

void QuadComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
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

		return;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(ShaderPath, Shader_Macros /*macros*/, nullptr /*include*/, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShaderByteCode, &errorPixelCode);

	device->CreateVertexShader(
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		nullptr, &vertexShader);

	device->CreatePixelShader(
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
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	device->CreateInputLayout(
		inputElements,
		3,
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		&layout);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = device->CreateRasterizerState(&rastDesc, &rastState);

	D3D11_BUFFER_DESC constBufDesc = {};
	constBufDesc.ByteWidth = sizeof(OffsetColor);
	constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufDesc.MiscFlags = 0;
	constBufDesc.StructureByteStride = 0;

	device->CreateBuffer(&constBufDesc, nullptr, &constantBuffer);

	points[0].texCoord = DirectX::XMFLOAT2(1, 1);
	points[1].texCoord = DirectX::XMFLOAT2(-1, -1);
	points[2].texCoord = DirectX::XMFLOAT2(-1, 1);
	points[3].texCoord = DirectX::XMFLOAT2(1, -1);

	SetColors(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
}

void QuadComponent::Draw(ID3D11Device* device, ID3D11DeviceContext* context) {

	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(Vertex) * std::size(points);

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = points;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	ID3D11Buffer* vb;
	device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

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
	device->CreateBuffer(&indexBufDesc, &indexData, &ib);

	UINT strides[] = { 40 };
	UINT offsets[] = { 0 };

	context->RSSetState(rastState);

	context->IASetInputLayout(layout);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	context->VSSetShader(vertexShader, nullptr, 0);
	context->PSSetShader(pixelShader, nullptr, 0);
	context->VSSetConstantBuffers(0, 1, &constantBuffer);
	context->PSSetConstantBuffers(0, 1, &constantBuffer);
	context->DrawIndexed(6, 0, 0);
}

void QuadComponent::Update(float deltaTime)
{
	// If windows signals to end the application then exit out.
	if (game->Input->IsKeyDown(Keys::Right))
		offsetColor.offset.x += 0.05f;
	if (game->Input->IsKeyDown(Keys::Left))
		offsetColor.offset.x -= 0.05f;
	if (game->Input->IsKeyDown(Keys::Up))
		offsetColor.offset.y += 0.05f;
	if (game->Input->IsKeyDown(Keys::Down))
		offsetColor.offset.y -= 0.05f;
	if (game->Input->IsKeyDown(Keys::Down))
		offsetColor.color = DirectX::XMFLOAT4(1.0f, 0, 0, 0);

}

void QuadComponent::SetOffset(float x, float y, float z)
{
	offsetColor.offset = DirectX::XMFLOAT4(x, y, z, 0.0f);

	/*D3D11_MAPPED_SUBRESOURCE res = {};
	context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, &offsetColor, sizeof(OffsetColor));
	context->Unmap(constantBuffer, 0);*/
}

void QuadComponent::SetRotation(float rot)
{
	offsetColor.rotation.w = rot;
	/*
	D3D11_MAPPED_SUBRESOURCE res = {};
	context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, &offsetColor, sizeof(OffsetColor));
	context->Unmap(constantBuffer, 0);*/
}

void QuadComponent::SetColors(float r1, float g1, float b1, float r2, float g2, float b2, float r3, float g3, float b3, float r4, float g4, float b4)
{
	points[0].color = DirectX::XMFLOAT4(r1, g1, b1, 0);
	points[1].color = DirectX::XMFLOAT4(r2, g2, b2, 0);
	points[2].color = DirectX::XMFLOAT4(r3, g3, b3, 0);
	points[3].color = DirectX::XMFLOAT4(r4, g4, b4, 0);
}

void QuadComponent::SetPositions(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4)
{
	points[0].position = DirectX::XMFLOAT4(x1, y1, z1, 0);
	points[1].position = DirectX::XMFLOAT4(x2, y2, z2, 0);
	points[2].position = DirectX::XMFLOAT4(x3, y3, z3, 0);
	points[3].position = DirectX::XMFLOAT4(x4, y4, z4, 0);
}
