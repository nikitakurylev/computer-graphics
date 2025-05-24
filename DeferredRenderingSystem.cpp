#include "DeferredRenderingSystem.h"

DeferredRenderingSystem::DeferredRenderingSystem(CubeComponent* cubes) : RenderingSystem(cubes)
{
}

void DeferredRenderingSystem::Draw(DisplayWin32* display, std::vector<GameComponent*> Components, Matrix view_matrix, Matrix projection_matrix, LightsParams dynamicLights[10], CascadeData* cascadeData, Vector3 cam_world)
{
	const float color[4] = { 0,0,0,0 };
	const float skyColor[4] = { 0.054f, 0.149f, 0.49f,0 };
	int i;

	Context->ClearState();

	Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->RSSetState(rastState);
	
	SetColorSampler();
	RenderDepthMaps(cascadeData, Components, view_matrix, cam_world);
	
	Context->RSSetViewports(1, &viewport);

	//Bind render target view array and depth stencil buffer to output render pipeline
	Context->OMSetRenderTargets(BUFFER_COUNT, renderTargetViewArray, depthStencilView);

	Context->IASetInputLayout(layout);
	//Clear the render target buffers
	for (i = 0; i < BUFFER_COUNT; i++)
	{
		Context->ClearRenderTargetView(renderTargetViewArray[i], color);
	}

	//Clear the depth buffer
	Context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	Context->IASetInputLayout(layout);
	for (GameComponent* gameComponent : Components)
	{
		Render(gameComponent, view_matrix, projection_matrix, vertexShader, pixelShader, cam_world);
	}

	
	Context->OMSetRenderTargets(1, &RenderView, depthStencilView);
	Context->ClearRenderTargetView(RenderView, color);
	Context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	Context->RSSetState(rasterizer);
	float blend[4] = { 1,1,1, 1 };
	Context->OMSetBlendState(blendState, blend, 0xFFFFFFFF);
	Context->OMSetDepthStencilState(depthState, 0);

	Context->PSSetShaderResources(0, 1, &shaderResourceViewArray[0]);
	Context->PSSetShaderResources(1, 1, &shaderResourceViewArray[1]);
	Context->PSSetShaderResources(2, 1, &shaderResourceViewArray[2]);
	Context->PSSetShaderResources(3, 1, &shaderResourceViewArray[3]);
	SetShadowMaps(4);

	UpdateCascadeBuffer(cascadeData);

	ID3D11Buffer* nothing = 0;
	UINT stride = 32;
	UINT offset = 0;
	Context->VSSetShader(dirVertexShader, 0, 0);
	Context->PSSetShader(dirPixelShader, 0, 0);
	Context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	Context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	Context->Draw(3, 0);
	Context->VSSetShader(pointVertexShader, 0, 0);
	Context->PSSetShader(pointPixelShader, 0, 0);
	CascadeData pointLightData;
	pointLightData.view_pos = Vector4(cam_world);

	for (int i = 0; i < 5; i++)
	{
		pointLightData.color = dynamicLights[i].color;
		pointLightData.direction = dynamicLights[i].direction;
		pointLightData.k = dynamicLights[i].k;
		UpdateCascadeBuffer(&pointLightData);
		auto world_matrix = 
			Matrix::CreateScale(dynamicLights[i].k.x * 2 * Vector3::One) *
			Matrix::CreateTranslation(Vector3(dynamicLights[i].direction)) *
			Matrix::Identity;
		UpdateTransformBuffer(world_matrix, view_matrix, projection_matrix, cam_world);
		UINT strides[] = { sizeof(Vertex) };
		UINT offsets[] = { 0 };
		Context->IASetVertexBuffers(0, 1, &sphereVertexBuffer, strides, offsets);
		Context->IASetIndexBuffer(sphereIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		Context->DrawIndexed(2400, 0, 0);
	}

	Context->VSSetShader(spotVertexShader, 0, 0);
	Context->PSSetShader(spotPixelShader, 0, 0);

	for (int i = 5; i < 10; i++)
	{
		pointLightData.color = dynamicLights[i].color;
		pointLightData.direction = dynamicLights[i].direction;
		pointLightData.k = dynamicLights[i].k;
		UpdateCascadeBuffer(&pointLightData);
		auto world_matrix =
			Matrix::CreateScale(dynamicLights[i].k.x * 2 * Vector3::One) *
			Matrix::CreateFromYawPitchRoll(0, i - 5, i - 5) *
			Matrix::CreateTranslation(Vector3(dynamicLights[i].direction)) *
			Matrix::Identity;
		UpdateTransformBuffer(world_matrix, view_matrix, projection_matrix, cam_world);
		UINT strides[] = { sizeof(Vertex) };
		UINT offsets[] = { 0 };
		Context->IASetVertexBuffers(0, 1, &coneVertexBuffer, strides, offsets);
		Context->IASetIndexBuffer(coneIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		Context->DrawIndexed(117, 0, 0);
	}

	Context->RSSetState(NULL);
	Context->OMSetBlendState(NULL, blend, 0xFFFFFFFF);
	Context->OMSetDepthStencilState(NULL, 0);

	SwapChain->Present(0, 0);
}

void DeferredRenderingSystem::Initialize(DisplayWin32* Display)
{
	vertexShaderName = L"./Shaders/DeferredVertexShader.hlsl";
	pixelShaderName = L"./Shaders/DeferredPixelShader.hlsl";
	RenderingSystem::Initialize(Display);

	ID3DBlob* vertexShaderByteCode = nullptr;
	ID3DBlob* pixelShaderByteCode = nullptr;
	ID3DBlob* pointVertexShaderByteCode = nullptr;
	ID3DBlob* pointPixelShaderByteCode = nullptr;
	ID3DBlob* spotVertexShaderByteCode = nullptr;
	ID3DBlob* spotPixelShaderByteCode = nullptr;
	
	auto res = CompileShaderFromFile(L"./Shaders/DirVertexShader.hlsl", 0, "main", "vs_4_0", &vertexShaderByteCode);
	res = CompileShaderFromFile(L"./Shaders/DirPixelShader.hlsl", 0, "main", "ps_4_0", &pixelShaderByteCode);
	res = CompileShaderFromFile(L"./Shaders/PointVertexShader.hlsl", 0, "main", "vs_4_0", &pointVertexShaderByteCode);
	res = CompileShaderFromFile(L"./Shaders/PointPixelShader.hlsl", 0, "main", "ps_4_0", &pointPixelShaderByteCode);
	res = CompileShaderFromFile(L"./Shaders/SpotVertexShader.hlsl", 0, "main", "vs_4_0", &spotVertexShaderByteCode);
	res = CompileShaderFromFile(L"./Shaders/SpotPixelShader.hlsl", 0, "main", "ps_4_0", &spotPixelShaderByteCode);
	
	Device->CreateVertexShader(
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		nullptr, &dirVertexShader);

	Device->CreatePixelShader(
		pixelShaderByteCode->GetBufferPointer(),
		pixelShaderByteCode->GetBufferSize(),
		nullptr, &dirPixelShader);

	Device->CreateVertexShader(
		pointVertexShaderByteCode->GetBufferPointer(),
		pointVertexShaderByteCode->GetBufferSize(),
		nullptr, &pointVertexShader);

	Device->CreatePixelShader(
		pointPixelShaderByteCode->GetBufferPointer(),
		pointPixelShaderByteCode->GetBufferSize(),
		nullptr, &pointPixelShader);

	Device->CreateVertexShader(
		spotVertexShaderByteCode->GetBufferPointer(),
		spotVertexShaderByteCode->GetBufferSize(),
		nullptr, &spotVertexShader);

	Device->CreatePixelShader(
		spotPixelShaderByteCode->GetBufferPointer(),
		spotPixelShaderByteCode->GetBufferSize(),
		nullptr, &spotPixelShader);

	SetDefferedSetup(Display->ClientWidth, Display->ClientHeight);
}

void DeferredRenderingSystem::SetDefferedSetup(int textureWidth, int textureHeight)
{
	int i;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;


	ID3D11Texture2D* renderTargetTextureArray[BUFFER_COUNT];

	for (i = 0; i < BUFFER_COUNT; i++)
	{
		Device->CreateTexture2D(&textureDesc, NULL, &renderTargetTextureArray[i]);

	}

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc32;
	ZeroMemory(&renderTargetViewDesc32, sizeof(renderTargetViewDesc32));
	renderTargetViewDesc32.Format = textureDesc.Format;
	renderTargetViewDesc32.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc32.Texture2D.MipSlice = 0;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc8;
	ZeroMemory(&renderTargetViewDesc8, sizeof(renderTargetViewDesc8));
	renderTargetViewDesc8.Format = textureDesc.Format;
	renderTargetViewDesc8.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc8.Texture2D.MipSlice = 0;


	//Create Render target view
	Device->CreateRenderTargetView(renderTargetTextureArray[0], &renderTargetViewDesc32, &renderTargetViewArray[0]);
	Device->CreateRenderTargetView(renderTargetTextureArray[1], &renderTargetViewDesc32, &renderTargetViewArray[1]);
	Device->CreateRenderTargetView(renderTargetTextureArray[2], &renderTargetViewDesc32, &renderTargetViewArray[2]);
	Device->CreateRenderTargetView(renderTargetTextureArray[3], &renderTargetViewDesc32, &renderTargetViewArray[3]);


	//Shader Resource View Description
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (i = 0; i < BUFFER_COUNT; i++)
	{
		Device->CreateShaderResourceView(renderTargetTextureArray[i], &shaderResourceViewDesc, &shaderResourceViewArray[i]);
	}

	//Release render target texture array
	for (i = 0; i < BUFFER_COUNT; i++)
	{
		renderTargetTextureArray[i]->Release();
	}

	//Depth Buffer Description
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = textureWidth;
	depthBufferDesc.Height = textureHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	Device->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);

	//Depth Stencil Description
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	Device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);


	//Rasterizer setup
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.DepthClipEnable = false;

	Device->CreateRasterizerState(&rasterizerDesc, &rasterizer);

	//Blend state setup
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	Device->CreateBlendState(&blendDesc, &blendState);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	Device->CreateDepthStencilState(&depthStencilDesc, &depthState);

	Vertex points[420];
	int indeces[2400];
	constexpr float thau = 6.28318530718f;
	for (auto j = 0; j <= 20; j++)
	{
		const auto height = static_cast<float>(j) / 20.0f;
		const auto t = sqrtf(height * (1.0f - height));

		for (auto i = 0; i < 20; i++)
		{
			float sin, cos;

			DirectX::XMScalarSinCos(&sin, &cos, thau * static_cast<float>(i) / 20.0f);

			const auto position = DirectX::XMFLOAT3(cos * t, height - 0.5f, sin * t);

			points[j * 20 + i].position = DirectX::XMFLOAT3(position.x, position.y, position.z);
			auto norm = Vector3(position.x, position.y, position.z);
			norm.Normalize();
			points[j * 20 + i].normal = -norm;
			points[j * 20 + i].texCoord = Vector2(0, 0);
		}
	}

	for (auto j = 0; j < 20; j++)
		for (auto i = 0; i < 20; i++)
		{
			const int index = j * 20 + i;

			indeces[6 * index] = index;
			indeces[6 * index + 1] = index + 20 - 1;
			indeces[6 * index + 2] = index + 20;
			indeces[6 * index + 3] = index + 20;
			indeces[6 * index + 4] = index + 1;
			indeces[6 * index + 5] = index;
		}

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

	Device->CreateBuffer(&vertexBufDesc, &vertexData, &sphereVertexBuffer);

	auto indexCount = std::size(indeces);
	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * indexCount;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indeces;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	Device->CreateBuffer(&indexBufDesc, &indexData, &sphereIndexBuffer);

	Vertex conePoints[21];
	int coneIndeces[117];
	conePoints[0].position = Vector3::Zero;

	for (auto i = 1; i < 21; i++)
	{
		float sin, cos;

		DirectX::XMScalarSinCos(&sin, &cos, thau * static_cast<float>(i) / 20.0f);

		const auto position = DirectX::XMFLOAT3(cos, -1, sin);

		conePoints[i].position = DirectX::XMFLOAT3(position.x, position.y, position.z);
		auto norm = Vector3(position.x, position.y, position.z);
		norm.Normalize();
		conePoints[i].normal = -norm;
		conePoints[i].texCoord = Vector2(0, 0);
	}

	for (auto i = 0; i < 20; i++)
	{
		coneIndeces[3 * i + 2] = 0;
		coneIndeces[3 * i + 1] = i;
		coneIndeces[3 * i] = i + 1;
	}

	for (auto i = 0; i < 18; i++)
	{
		coneIndeces[63 + 3 * i] = 1;
		coneIndeces[63 + 3 * i + 1] = i + 2;
		coneIndeces[63 + 3 * i + 2] = i + 3;
	}

	coneIndeces[62] = 0;
	coneIndeces[61] = 20;
	coneIndeces[60] = 1;

	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(Vertex) * std::size(conePoints);

	vertexData.pSysMem = conePoints;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	Device->CreateBuffer(&vertexBufDesc, &vertexData, &coneVertexBuffer);

	indexCount = std::size(coneIndeces);
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * indexCount;

	indexData.pSysMem = coneIndeces;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	Device->CreateBuffer(&indexBufDesc, &indexData, &coneIndexBuffer);
}
