#include <algorithm>
#include <cmath>
#include <iostream>
#include "RenderingSystem.h"
#include "GameObject.h"
#include "CubeComponent.h"
#include "Renderer.h"
#include "ParticleSystemComponent.h"
#include "PointLightComponent.h"
#include "DisplayWin32.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

void RenderingSystem::Draw(DisplayWin32* display, std::vector<GameObject*> Components, Matrix view_matrix, Matrix projection_matrix, CascadeData* cascadeData, Vector3 cam_world)
{
	Context->ClearState();
	Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->RSSetState(rastState);

	SetColorSampler();
	RenderDepthMaps(cascadeData, Components, view_matrix, cam_world);

	float color[] = { 0.054f, 0.149f, 0.49f };
	Context->ClearRenderTargetView(RenderView, color);
	Context->ClearDepthStencilView(depth_stencil_view[3], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	Context->OMSetRenderTargets(1, &RenderView, depth_stencil_view[3]);
	Context->RSSetViewports(1, &viewport);

	memset(dynamicLights, 0, sizeof(dynamicLights));
	int lightIndex = 0;
	for (GameObject* gameObject : Components) {
		if (lightIndex >= 10) break;

		for (Component* component : gameObject->GetComponents()) {
			auto* pointLight = dynamic_cast<PointLightComponent*>(component);
			if (pointLight) {
				auto transform = gameObject->GetTransform();
				dynamicLights[lightIndex].dyn_position = Vector4(transform->GetMatrix().Translation());
				dynamicLights[lightIndex].dyn_color = pointLight->color;
				dynamicLights[lightIndex].dyn_k = Vector4(pointLight->radius, 100.0f, 1.2f, 0.0f);
				lightIndex++;
				break;
			}
		}
	}

	if (dynamicLightBuffer != nullptr) {
		Context->UpdateSubresource(dynamicLightBuffer, 0, nullptr, dynamicLights, 0, 0);
	}

	Context->VSSetConstantBuffers(1, 1, &lightTransformBuffer);
	Context->PSSetConstantBuffers(1, 1, &dynamicLightBuffer);
	UpdateCascadeBuffer(cascadeData);
	SetShadowMaps(5);

	// Extract frustum planes from ViewProjection matrix
	Matrix viewProj = view_matrix * projection_matrix;
	manualFrustumCuller.ExtractPlanes(viewProj);

	// Reset debug counters
	debugRenderedCount = 0;
	debugCulledCount = 0;
	debugTotalCount = 0;

	for (GameObject* gameComponent : Components)
	{
		Render(gameComponent, view_matrix, projection_matrix, vertexShader, pixelShader, cam_world, true);
	}

	Context->OMSetRenderTargets(0, nullptr, nullptr);
	SwapChain->Present(1, 0);
}

void RenderingSystem::RenderDepthMaps(CascadeData* cascadeData, std::vector<GameObject*> Components, const Matrix& view_matrix, Vector3 cam_world) {
	RenderDepthMap(0, cascadeData, Components, view_matrix, cam_world);
	RenderDepthMap(1, cascadeData, Components, view_matrix, cam_world);
	RenderDepthMap(2, cascadeData, Components, view_matrix, cam_world);
	RenderDepthMap(3, cascadeData, Components, view_matrix, cam_world);
}

void RenderingSystem::SetShadowMaps(int offset) {
	Context->PSSetSamplers(1, 1, &DepthSamplerState);
	Context->PSSetShaderResources(offset, 1, &resource_view_depth_directional_light[0]);
	Context->PSSetShaderResources(offset + 1, 1, &resource_view_depth_directional_light[1]);
	Context->PSSetShaderResources(offset + 2, 1, &resource_view_depth_directional_light[2]);
	Context->PSSetShaderResources(offset + 3, 1, &resource_view_depth_directional_light[3]);
}

void RenderingSystem::UpdateCascadeBuffer(CascadeData* cascadeData) {
	Context->UpdateSubresource(lightTransformBuffer, 0, nullptr, cascadeData, 0, 0);
	Context->PSSetConstantBuffers(0, 1, &lightTransformBuffer);
}

void RenderingSystem::SetColorSampler() {
	Context->PSSetSamplers(0, 1, &TexSamplerState);
	Context->CSSetSamplers(0, 1, &TexSamplerState);
}

void RenderingSystem::InitEnvironmentResources() {
	if (environmentMap_ != nullptr) {
		return;
	}

	struct ColorRGBA {
		unsigned char r, g, b, a;
	};

	const ColorRGBA sky = { 135, 206, 235, 255 };
	const ColorRGBA ground = { 20, 30, 20, 255 };

	ColorRGBA faceColors[6] = {
		sky,
		sky,
		sky,
		ground,
		sky,
		sky
	};

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SUBRESOURCE_DATA initData[6] = {};
	for (int i = 0; i < 6; i++) {
		initData[i].pSysMem = &faceColors[i];
		initData[i].SysMemPitch = sizeof(ColorRGBA);
	}

	ID3D11Texture2D* cubeTexture = nullptr;
	if (FAILED(Device->CreateTexture2D(&desc, initData, &cubeTexture))) {
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;

	Device->CreateShaderResourceView(cubeTexture, &srvDesc, &environmentMap_);
	cubeTexture->Release();
}

void RenderingSystem::BindEnvironmentResources(int slot) {
	if (environmentMap_ != nullptr) {
		Context->PSSetShaderResources(slot, 1, &environmentMap_);
	}
}

void RenderingSystem::InitDefaultMaterialResources() {
	if (defaultWhiteTexture_ != nullptr && defaultNormalTexture_ != nullptr && defaultMaterialBuffer_ != nullptr) {
		return;
	}

	const UINT whitePixel = 0xFFFFFFFF;
	const unsigned char normalPixel[4] = { 128, 128, 255, 255 };
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = &whitePixel;
	subresourceData.SysMemPitch = sizeof(UINT);

	ID3D11Texture2D* texture2D = nullptr;
	if (SUCCEEDED(Device->CreateTexture2D(&desc, &subresourceData, &texture2D))) {
		Device->CreateShaderResourceView(texture2D, nullptr, &defaultWhiteTexture_);
		texture2D->Release();
	}

	subresourceData.pSysMem = normalPixel;
	subresourceData.SysMemPitch = 4;

	texture2D = nullptr;
	if (SUCCEEDED(Device->CreateTexture2D(&desc, &subresourceData, &texture2D))) {
		Device->CreateShaderResourceView(texture2D, nullptr, &defaultNormalTexture_);
		texture2D->Release();
	}

	MaterialBufferData materialData = {};
	materialData.baseColorFactor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData.materialParams = Vector4(0.0f, 1.0f, 1.0f, 0.0f);

	D3D11_BUFFER_DESC mbd = {};
	mbd.ByteWidth = sizeof(MaterialBufferData);
	mbd.Usage = D3D11_USAGE_DEFAULT;
	mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mbd.CPUAccessFlags = 0;
	mbd.MiscFlags = 0;
	mbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = &materialData;

	Device->CreateBuffer(&mbd, &initData, &defaultMaterialBuffer_);
}

void RenderingSystem::BindDefaultMaterialResources() {
	if (defaultMaterialBuffer_ != nullptr) {
		Context->PSSetConstantBuffers(2, 1, &defaultMaterialBuffer_);
	}
	if (defaultWhiteTexture_ != nullptr) {
		ID3D11ShaderResourceView* normalTexture = defaultNormalTexture_ ? defaultNormalTexture_ : defaultWhiteTexture_;
		ID3D11ShaderResourceView* srvs[] = { defaultWhiteTexture_, defaultWhiteTexture_, defaultWhiteTexture_, defaultWhiteTexture_, normalTexture };
		Context->PSSetShaderResources(0, 5, srvs);
	}
}

void RenderingSystem::Render(GameObject* gameObject, Matrix view, Matrix projection, ID3D11VertexShader* vertex, ID3D11PixelShader* pixel, Vector3 cam_world, bool culling, bool drawDebugAABB)
{
	auto world_matrix = gameObject->GetTransform()->GetMatrix();
	for (Component* gameComponent : gameObject->GetComponents()) {
		auto renderer = dynamic_cast<Renderer*>(gameComponent);
		if (!renderer)
			continue;

		if (culling) {
			debugTotalCount++;
		}

		bool isCulled = false;

		if (culling) {
			BoundingBox aabb;
			bool hasAABB = renderer->GetGlobalAABB(aabb);

			if (hasAABB) {
				Vector3 extent = aabb.Extents;
				bool valid = extent.x > 0.0f && extent.y > 0.0f && extent.z > 0.0f &&
					!std::isnan(extent.x) && !std::isnan(extent.y) && !std::isnan(extent.z);

				if (valid) {
					bool intersects = manualFrustumCuller.IntersectsAABB(aabb);

					if (drawDebugAABB && debugShowAABB) {
						RenderDebugAABB(aabb, view, projection, cam_world, !intersects);
					}

					if (!intersects) {
						isCulled = true;
						debugCulledCount++;
						continue;
					}
				}
			}
		}

		if (culling) {
			debugRenderedCount++;
		}
		UpdateTransformBuffer(world_matrix, view, projection, cam_world);
		Context->VSSetShader(vertex, 0, 0);
		Context->PSSetShader(pixel, 0, 0);
		BindDefaultMaterialResources();
		renderer->Draw(Device, Context);
	}
}

void RenderingSystem::UpdateTransformBuffer(Matrix world_matrix, Matrix view, Matrix projection, Vector3 cam_world) {
	ConstantBuffer buffer;
	buffer.ViewProjection = world_matrix * view * projection;
	buffer.World = world_matrix;
	buffer.ViewPosition = Vector4(cam_world);
	buffer.InverseProjectionView = buffer.ViewProjection.Invert();
	buffer.ViewInv = view.Invert();
	buffer.ProjInv = projection.Invert();
	buffer.View = view;
	buffer.Projection = projection;
	Context->UpdateSubresource(constantBuffer, 0, nullptr, &buffer, 0, 0);
	Context->VSSetConstantBuffers(0, 1, &constantBuffer);
	Context->GSSetConstantBuffers(0, 1, &constantBuffer);
	Context->CSSetConstantBuffers(0, 1, &constantBuffer);
}

RenderingSystem::RenderingSystem(DisplayWin32* Display, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName) : vertexShaderName(vertexShaderName), pixelShaderName(pixelShaderName)
{
	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Width = Display->ClientWidth;
	swapDesc.BufferDesc.Height = Display->ClientHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = Display->hWnd;
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	auto res = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDesc,
		&SwapChain,
		&Device,
		nullptr,
		&Context);

	if (FAILED(res))
	{
		// device creation failed
	}

	ID3D11Texture2D* backTex;
	res = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);
	res = Device->CreateRenderTargetView(backTex, nullptr, &RenderView);

	ID3DBlob* vertexShaderByteCode = nullptr;
	ID3DBlob* errorVertexCode = nullptr;
	res = CompileShaderFromFile(vertexShaderName, 0, "main", "vs_4_0", &vertexShaderByteCode);

	ID3DBlob* pixelShaderByteCode;
	res = CompileShaderFromFile(pixelShaderName, 0, "main", "ps_4_0", &pixelShaderByteCode);
	Device->CreateVertexShader(
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		nullptr, &vertexShader);

	Device->CreatePixelShader(
		pixelShaderByteCode->GetBufferPointer(),
		pixelShaderByteCode->GetBufferSize(),
		nullptr, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC inputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	Device->CreateInputLayout(
		inputElements,
		5,
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		&layout);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = Device->CreateRasterizerState(&rastDesc, &rastState);

	D3D11_BUFFER_DESC constBufDesc = {};
	constBufDesc.ByteWidth = sizeof(ConstantBuffer);
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = 0;
	constBufDesc.MiscFlags = 0;
	constBufDesc.StructureByteStride = 0;

	Device->CreateBuffer(&constBufDesc, nullptr, &constantBuffer);

	constBufDesc = {};
	constBufDesc.ByteWidth = sizeof(CascadeData);
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = 0;
	constBufDesc.MiscFlags = 0;
	constBufDesc.StructureByteStride = 0;

	res = Device->CreateBuffer(&constBufDesc, nullptr, &lightTransformBuffer);

	// Dynamic lights buffer (10 point lights for forward rendering)
	D3D11_BUFFER_DESC dynamicLightBufDesc = {};
	dynamicLightBufDesc.ByteWidth = sizeof(PointLightData) * 10;
	dynamicLightBufDesc.Usage = D3D11_USAGE_DEFAULT;
	dynamicLightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dynamicLightBufDesc.CPUAccessFlags = 0;
	dynamicLightBufDesc.MiscFlags = 0;
	dynamicLightBufDesc.StructureByteStride = 0;

	res = Device->CreateBuffer(&dynamicLightBufDesc, nullptr, &dynamicLightBuffer);

	memset(dynamicLights, 0, sizeof(dynamicLights));

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	res = Device->CreateSamplerState(&samplerDesc, &TexSamplerState);
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	res = Device->CreateSamplerState(&samplerDesc, &DepthSamplerState);

	InitDefaultMaterialResources();
	InitEnvironmentResources();

	auto fov = DirectX::XM_PIDIV2;
	auto aspect = Display->ClientWidth / (FLOAT)Display->ClientHeight;

	directional_light_projection[0] = Matrix::CreatePerspectiveFieldOfView(fov, aspect, 0.01f, 10);
	directional_light_projection[1] = Matrix::CreatePerspectiveFieldOfView(fov, aspect, 10, 30);
	directional_light_projection[2] = Matrix::CreatePerspectiveFieldOfView(fov, aspect, 30, 60);
	directional_light_projection[3] = Matrix::CreatePerspectiveFieldOfView(fov, aspect, 60, 500);


	viewport = {};
	viewport.Width = static_cast<float>(Display->ClientWidth);
	viewport.Height = static_cast<float>(Display->ClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

	InitDepthMap(0, 0.5f, Display);
	InitDepthMap(1, 0.5f, Display);
	InitDepthMap(2, 0.25f, Display);
	InitDepthMap(3, 1.0f, Display);

	InitDebugLineRenderer();

	ID3DBlob* error_message;
	ID3DBlob* vertex_shader_buffer;
	ID3DBlob* pixel_shader_buffer;

	D3DCompileFromFile(L"./DepthShader.hlsl",
		nullptr, nullptr,
		"VSMain", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0,
		&vertex_shader_buffer, &error_message);

	D3DCompileFromFile(L"./DepthShader.hlsl",
		nullptr, nullptr,
		"PSMain", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0,
		&pixel_shader_buffer, &error_message);

	Device->CreateVertexShader(
		vertex_shader_buffer->GetBufferPointer(), vertex_shader_buffer->GetBufferSize(),
		nullptr, &depthVertexShader);

	Device->CreatePixelShader(
		pixel_shader_buffer->GetBufferPointer(), pixel_shader_buffer->GetBufferSize(),
		nullptr, &depthPixelShader);

	D3DCompileFromFile(L"./DebugShader.hlsl",
		nullptr, nullptr,
		"VSMain", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0,
		&vertex_shader_buffer, &error_message);

	D3DCompileFromFile(L"./DebugShader.hlsl",
		nullptr, nullptr,
		"PSMain", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0,
		&pixel_shader_buffer, &error_message);

	Device->CreateVertexShader(
		vertex_shader_buffer->GetBufferPointer(), vertex_shader_buffer->GetBufferSize(),
		nullptr, &debugVertexShader);

	Device->CreatePixelShader(
		pixel_shader_buffer->GetBufferPointer(), pixel_shader_buffer->GetBufferSize(),
		nullptr, &debugPixelShader);
}

void RenderingSystem::Initialize(std::vector<GameObject*> GameObjects)
{
	for (GameObject* gameObject : GameObjects) {
		for (Component* gameComponent : gameObject->GetComponents()) {
			auto renderer = dynamic_cast<Renderer*>(gameComponent);
			if (!renderer)
				continue;
			renderer->Initialize(Device, Context);
		}
	}
}

void RenderingSystem::InitDepthMap(int index, float resolution, DisplayWin32* Display)
{
	D3D11_TEXTURE2D_DESC depth_texture_descriptor{};
	depth_texture_descriptor.Width = Display->ClientWidth * resolution;
	depth_texture_descriptor.Height = Display->ClientHeight * resolution;
	depth_texture_descriptor.MipLevels = 1;
	depth_texture_descriptor.ArraySize = 1;
	depth_texture_descriptor.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	depth_texture_descriptor.SampleDesc.Count = 1;
	depth_texture_descriptor.Usage = D3D11_USAGE_DEFAULT;
	depth_texture_descriptor.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	viewport_depth_directional_light_[index] = viewport;
	viewport_depth_directional_light_[index].Width *= resolution;
	viewport_depth_directional_light_[index].Height *= resolution;

	ID3D11Texture2D* background_texture_depth;
	Device->CreateTexture2D(&depth_texture_descriptor, nullptr, &background_texture_depth);
	auto res = Device->CreateRenderTargetView(background_texture_depth, nullptr, &(render_target_view_depth_directional_light[index]));
	res = Device->CreateShaderResourceView(background_texture_depth, nullptr, &(resource_view_depth_directional_light[index]));

	D3D11_TEXTURE2D_DESC depth_stencil_descriptor;
	depth_stencil_descriptor.Width = Display->ClientWidth * resolution;
	depth_stencil_descriptor.Height = Display->ClientHeight * resolution;
	depth_stencil_descriptor.MipLevels = 1;
	depth_stencil_descriptor.ArraySize = 1;
	depth_stencil_descriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_descriptor.SampleDesc.Count = 1;
	depth_stencil_descriptor.SampleDesc.Quality = 0;
	depth_stencil_descriptor.Usage = D3D11_USAGE_DEFAULT;
	depth_stencil_descriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_stencil_descriptor.CPUAccessFlags = 0;
	depth_stencil_descriptor.MiscFlags = 0;

	res = Device->CreateTexture2D(&depth_stencil_descriptor, nullptr, &(depth_stencil_buffer[index]));
	res = Device->CreateDepthStencilView(depth_stencil_buffer[index], nullptr, &(depth_stencil_view[index]));
}

HRESULT RenderingSystem::CompileShaderFromFile(LPCWSTR pFileName, const D3D_SHADER_MACRO* pDefines, LPCSTR pEntryPoint, LPCSTR pShaderModel, ID3DBlob** ppBytecodeBlob)
{
	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob = nullptr;

	HRESULT result = D3DCompileFromFile(pFileName, pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, pEntryPoint, pShaderModel, compileFlags, 0, ppBytecodeBlob, &pErrorBlob);
	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
			OutputDebugStringA((LPCSTR)pErrorBlob->GetBufferPointer());
	}

	if (pErrorBlob != nullptr)
		pErrorBlob->Release();

	return result;
}

void RenderingSystem::RenderDepthMap(int index, CascadeData* cascadeData, std::vector<GameObject*> Components, const Matrix& view_matrix, Vector3 cam_world)
{
	auto corners = GetFrustrumCornersWorldSpace(directional_light_projection[index], view_matrix);
	auto view = GetCascadeView(corners, index, cascadeData->position);
	auto projection = GetCascadeProjection(view, corners, index);
	cascadeData->ViewProj[index] = view * projection;

	float black[] = { 0.0f, 0.0f, 0.0f };
	Context->ClearRenderTargetView(render_target_view_depth_directional_light[index], black);
	Context->ClearDepthStencilView(depth_stencil_view[index], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	Context->OMSetRenderTargets(1, &render_target_view_depth_directional_light[index], depth_stencil_view[index]);

	Context->RSSetViewports(1, &viewport_depth_directional_light_[index]);

	Context->IASetInputLayout(layout);

	for (GameObject* gameComponent : Components)
	{
		Render(gameComponent, view, projection, depthVertexShader, depthPixelShader, cam_world, false);
	}

	Context->OMSetRenderTargets(1, &RenderView, nullptr);
}

std::vector<Vector4> RenderingSystem::GetFrustrumCornersWorldSpace(const Matrix& proj, const Matrix& view_matrix)
{
	const auto viewProj = view_matrix * proj;
	const auto inv = viewProj.Invert();

	std::vector<Vector4> frustrumCorners;
	frustrumCorners.reserve(8);
	for (int x = 0; x < 2; x++)
		for (int y = 0; y < 2; y++)
			for (int z = 0; z < 2; z++) {
				const Vector4 pt =
					Vector4::Transform(Vector4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						z,
						1.0f), inv);
				frustrumCorners.push_back(pt / pt.w);
			}

	return frustrumCorners;
}

Matrix RenderingSystem::GetCascadeView(const std::vector<Vector4>& corners, int index, Vector4 direction)
{
	Vector3 center = Vector3::Zero;

	for (const auto& v : corners)
		center += Vector3(v.x, v.y, v.z);

	center /= corners.size();

	const auto lightView = Matrix::CreateLookAt(
		center,
		center + direction,
		Vector3::Up
	);

	return lightView;
}

Matrix RenderingSystem::GetCascadeProjection(const Matrix& lightView, const std::vector<Vector4>& corners, int index)
{
	float minX = FLT_MAX;
	float maxX = -FLT_MAX;
	float minY = FLT_MAX;
	float maxY = -FLT_MAX;
	float minZ = FLT_MAX;
	float maxZ = -FLT_MAX;

	for (const auto& v : corners)
	{
		const auto trf = Vector4::Transform(v, lightView);

		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}

	constexpr float zMult = 10.0f;
	minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
	maxZ = (maxZ < 0) ? maxZ / zMult : maxZ * zMult;

	auto lightProjection = Matrix::CreateOrthographicOffCenter(minX, maxX, minY, maxY, minZ, maxZ);

	return lightProjection;
}

void RenderingSystem::InitDebugLineRenderer()
{
	ID3DBlob* vertexBC = nullptr;
	ID3DBlob* pixelBC = nullptr;

	HRESULT hr1 = CompileShaderFromFile(L"./SimpleTexturedDirectx11/DebugLineVertexShader.hlsl", nullptr, "main", "vs_5_0", &vertexBC);
	HRESULT hr2 = CompileShaderFromFile(L"./SimpleTexturedDirectx11/DebugLinePixelShader.hlsl", nullptr, "main", "ps_5_0", &pixelBC);

	if (FAILED(hr1) || FAILED(hr2)) {
		return;
	}

	Device->CreateVertexShader(vertexBC->GetBufferPointer(), vertexBC->GetBufferSize(), nullptr, &debugLineVertexShader);
	Device->CreatePixelShader(pixelBC->GetBufferPointer(), pixelBC->GetBufferSize(), nullptr, &debugLinePixelShader);

	D3D11_INPUT_ELEMENT_DESC debugLineLayout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	Device->CreateInputLayout(debugLineLayout, 2, vertexBC->GetBufferPointer(), vertexBC->GetBufferSize(), &debugLineInputLayout);

	if (vertexBC) vertexBC->Release();
	if (pixelBC) pixelBC->Release();

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(float) * 7 * 48;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	Device->CreateBuffer(&bufferDesc, nullptr, &debugLineVertexBuffer);
}

void RenderingSystem::RenderDebugAABB(const BoundingBox& aabb, const Matrix& view, const Matrix& projection, const Vector3& cam_world, bool isCulled)
{
	if (!debugLineVertexBuffer) return;

	ID3D11InputLayout* oldLayout = nullptr;
	ID3D11Buffer* oldVertexBuffer = nullptr;
	ID3D11Buffer* oldIndexBuffer = nullptr;
	ID3D11RenderTargetView* oldRenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
	ID3D11DepthStencilView* oldDepthStencil = nullptr;
	DXGI_FORMAT oldIndexFormat;
	UINT oldStride = 0, oldOffset = 0, oldIndexOffset = 0;
	ID3D11VertexShader* oldVS = nullptr;
	ID3D11PixelShader* oldPS = nullptr;
	ID3D11RasterizerState* oldRS = nullptr;
	D3D11_PRIMITIVE_TOPOLOGY oldTopology;

	Context->IAGetInputLayout(&oldLayout);
	Context->IAGetVertexBuffers(0, 1, &oldVertexBuffer, &oldStride, &oldOffset);
	Context->IAGetIndexBuffer(&oldIndexBuffer, &oldIndexFormat, &oldIndexOffset);
	Context->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, oldRenderTargets, &oldDepthStencil);
	Context->VSGetShader(&oldVS, nullptr, nullptr);
	Context->PSGetShader(&oldPS, nullptr, nullptr);
	Context->RSGetState(&oldRS);
	Context->IAGetPrimitiveTopology(&oldTopology);

	XMFLOAT3 corners[8];
	aabb.GetCorners(corners);

	Vector4 color = isCulled ? Vector4(1.0f, 0.0f, 0.0f, 1.0f) : Vector4(0.0f, 1.0f, 0.0f, 1.0f);

	struct LineVertex {
		Vector3 position;
		Vector4 color;
	};

	LineVertex lines[24];
	int idx = 0;

	int edges[12][2] = {
		{0,1}, {1,3}, {3,2}, {2,0},
		{4,5}, {5,7}, {7,6}, {6,4},
		{0,4}, {1,5}, {2,6}, {3,7}
	};

	for (int i = 0; i < 12; i++) {
		lines[idx].position = Vector3(corners[edges[i][0]].x, corners[edges[i][0]].y, corners[edges[i][0]].z);
		lines[idx].color = color;
		idx++;

		lines[idx].position = Vector3(corners[edges[i][1]].x, corners[edges[i][1]].y, corners[edges[i][1]].z);
		lines[idx].color = color;
		idx++;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (SUCCEEDED(Context->Map(debugLineVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
		memcpy(mappedResource.pData, lines, sizeof(lines));
		Context->Unmap(debugLineVertexBuffer, 0);
	}

	UpdateTransformBuffer(Matrix::Identity, view, projection, cam_world);

	UINT stride = sizeof(LineVertex);
	UINT offset = 0;

	Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	Context->IASetInputLayout(debugLineInputLayout);
	Context->IASetVertexBuffers(0, 1, &debugLineVertexBuffer, &stride, &offset);
	Context->VSSetShader(debugLineVertexShader, 0, 0);
	Context->PSSetShader(debugLinePixelShader, 0, 0);

	Context->Draw(24, 0);

	Context->IASetPrimitiveTopology(oldTopology);
	Context->IASetInputLayout(oldLayout);
	Context->IASetVertexBuffers(0, 1, &oldVertexBuffer, &oldStride, &oldOffset);
	Context->IASetIndexBuffer(oldIndexBuffer, oldIndexFormat, oldIndexOffset);
	Context->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, oldRenderTargets, oldDepthStencil);
	Context->VSSetShader(oldVS, nullptr, 0);
	Context->PSSetShader(oldPS, nullptr, 0);
	Context->RSSetState(oldRS);

	if (oldLayout) oldLayout->Release();
	if (oldVertexBuffer) oldVertexBuffer->Release();
	if (oldIndexBuffer) oldIndexBuffer->Release();
	if (oldDepthStencil) oldDepthStencil->Release();
	for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
		if (oldRenderTargets[i]) oldRenderTargets[i]->Release();
	}
	if (oldVS) oldVS->Release();
	if (oldPS) oldPS->Release();
	if (oldRS) oldRS->Release();
}
