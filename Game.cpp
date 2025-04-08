#include "Game.h"
#include <d3d11.h>
#include <iostream>
#include "GameComponent.h"
#include <d3dcompiler.h>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

Game::Game(DisplayWin32* display, InputDevice* input) : Display(display), Input(input)
{
}

void Game::Run()
{
	unsigned int frameCount = 0;


	MSG msg = {};
	bool isExitRequested = false;
	auto t = 0.0f;
	while (!isExitRequested) {
		// Handle the windows messages.
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT) {
			isExitRequested = true;
		}

		auto	curTime = std::chrono::steady_clock::now();
		float	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;
		t += deltaTime;
		TotalTime += deltaTime;
		frameCount++;

		if (TotalTime > 1.0f) {
			float fps = frameCount / TotalTime;

			TotalTime -= 1.0f;

			WCHAR text[256];
			swprintf_s(text, TEXT("FPS: %f"), fps);
			SetWindowText(Display->hWnd, text);

			frameCount = 0;
		}

		if(Input->IsKeyDown(Keys::D1))
			fps = true;
		else if (Input->IsKeyDown(Keys::D2))
			fps = false;

		if (Input->IsKeyDown(Keys::D3)) {
			ortho = false;
			projection_matrix = Matrix::CreatePerspectiveFieldOfView(
				DirectX::XM_PIDIV2, Display->ClientWidth / (FLOAT)Display->ClientHeight,
				0.01f, 1000);
		}
		else if (Input->IsKeyDown(Keys::D4)) {
			ortho = true;
			projection_matrix = Matrix::CreateOrthographic(Display->ClientWidth * distance * 0.001f, Display->ClientHeight * distance * 0.001f, 0.01f, 1000);
		}

		cam_rot.y += Input->MouseOffset.x * 0.01f;
		cam_rot.x += Input->MouseOffset.y * 0.01f;
		Input->MouseOffset = Vector2();

		if (fps) {
			auto camera_matrix =
				Matrix::CreateFromYawPitchRoll(cam_rot);
				//* Matrix::CreateTranslation(cam_pos);

			if (Input->IsKeyDown(Keys::W))
				cam_pos += camera_matrix.Forward() * 0.01f;
			if (Input->IsKeyDown(Keys::S))
				cam_pos -= camera_matrix.Forward() * 0.01f;
			if (Input->IsKeyDown(Keys::D))
				cam_pos += camera_matrix.Right() * 0.01f;
			if (Input->IsKeyDown(Keys::A))
				cam_pos -= camera_matrix.Right() * 0.01f;
			if (Input->IsKeyDown(Keys::Space))
				cam_pos.y += 0.01f;
			if (Input->IsKeyDown(Keys::LeftShift))
				cam_pos.y -= 0.01f;
			const auto rotation = Matrix::CreateFromYawPitchRoll(cam_rot);
			const auto target = Vector3::Transform(Vector3::Forward, rotation) + Vector3(cam_pos);
			const auto up_direction = Vector3::Transform(Vector3::Up, rotation);
			view_matrix = Matrix::CreateLookAt(Vector3(cam_pos), target, up_direction);
		}
		else {
			distance = max(1.0f, distance - Input->MouseWheelDelta * 0.01f);
			if(ortho && Input-> MouseWheelDelta != 0)
				projection_matrix = Matrix::CreateOrthographic(Display->ClientWidth * distance * 0.001f, Display->ClientHeight * distance * 0.001f, 0.01f, 1000);
			Input->MouseWheelDelta = 0;
			auto lookAtPoint = cam_pos;
			cam_world = Vector3(distance, 0, 0); // distance - расстояние от камеры
			// до точки просмотра
			Matrix rotMat = Matrix::CreateFromYawPitchRoll(Vector3(0, -cam_rot.y, cam_rot.x));
			cam_world = Vector3::Transform(cam_world, rotMat) + lookAtPoint; // Финальная позиция камеры
			view_matrix = Matrix::CreateLookAt(cam_world, lookAtPoint, Vector3::Transform(Vector3::Up, rotMat));
		}

		Update(deltaTime);
		Draw();
	}

	std::cout << "Hello World!\n";
}

void Game::Update(float deltaTime)
{
	for (GameComponent* gameComponent : Components)
	{
		gameComponent->Update(deltaTime);
	}
}

void Game::Draw()
{
	Context->ClearState();


	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(Display->ClientWidth);
	viewport.Height = static_cast<float>(Display->ClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

	Context->RSSetViewports(1, &viewport);

	Context->OMSetRenderTargets(1, &RenderView, nullptr);

	float color[] = { 0.054f, 0.149f, 0.49f };
	Context->ClearRenderTargetView(RenderView, color);
	Context->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	Context->OMSetRenderTargets(1, &RenderView, depth_stencil_view_);
	Context->RSSetState(rastState);
	Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Context->IASetInputLayout(layout);


	for (GameComponent* gameComponent : Components)
	{
		ConstantBuffer buffer;
		buffer.View = gameComponent->world_matrix * view_matrix * projection_matrix;
		buffer.World = gameComponent->world_matrix;
		buffer.ViewPosition = Vector4(cam_world);
		Context->UpdateSubresource(constantBuffer, 0, nullptr, &buffer, 0, 0);
		Context->UpdateSubresource(lightBuffer, 0, nullptr, &light, 0, 0);
		Context->UpdateSubresource(dynamicLightBuffer, 0, nullptr, dynamicLights, 0, 0);
		Context->VSSetConstantBuffers(0, 1, &constantBuffer);
		Context->PSSetConstantBuffers(0, 1, &constantBuffer);
		Context->PSSetConstantBuffers(1, 1, &lightBuffer);
		Context->PSSetConstantBuffers(2, 1, &dynamicLightBuffer);
		Context->PSSetSamplers(0, 1, &TexSamplerState);
		gameComponent->Draw();
	}

	Context->OMSetRenderTargets(0, nullptr, nullptr);

	SwapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
}

Matrix Game::GetCameraMatrix()
{
	return Matrix::CreateFromYawPitchRoll(Vector3(0, -cam_rot.y, cam_rot.x));
}

void Game::Initialize()
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
		// Well, that was unexpected
	}


	ID3D11Texture2D* backTex;
	res = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);	// __uuidof(ID3D11Texture2D)
	res = Device->CreateRenderTargetView(backTex, nullptr, &RenderView);

	PrevTime = std::chrono::steady_clock::now();
	TotalTime = 0;
	

	D3D11_TEXTURE2D_DESC depth_stencil_descriptor;
	depth_stencil_descriptor.Width = Display->ClientWidth;
	depth_stencil_descriptor.Height = Display->ClientHeight;
	depth_stencil_descriptor.MipLevels = 1;
	depth_stencil_descriptor.ArraySize = 1;
	depth_stencil_descriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_descriptor.SampleDesc.Count = 1;
	depth_stencil_descriptor.SampleDesc.Quality = 0;
	depth_stencil_descriptor.Usage = D3D11_USAGE_DEFAULT;
	depth_stencil_descriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_stencil_descriptor.CPUAccessFlags = 0;
	depth_stencil_descriptor.MiscFlags = 0;

	Device->CreateTexture2D(&depth_stencil_descriptor, nullptr, &depth_stencil_buffer_);
	Device->CreateDepthStencilView(depth_stencil_buffer_, nullptr, &depth_stencil_view_);
	Context->OMSetRenderTargets(1, &RenderView, depth_stencil_view_);


	projection_matrix = Matrix::CreatePerspectiveFieldOfView(
		DirectX::XM_PIDIV2, Display->ClientWidth/(FLOAT)Display->ClientHeight,
		0.01f, 1000);

	vertexShaderByteCode = nullptr;
	ID3DBlob* errorVertexCode = nullptr;
	res = CompileShaderFromFile(L"./SimpleTexturedDirectx11/VertexShader.hlsl", 0, "main", "vs_4_0", &vertexShaderByteCode);


	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode;
	res = CompileShaderFromFile(L"./SimpleTexturedDirectx11/PixelShader.hlsl", 0, "main", "ps_4_0", &pixelShaderByteCode);
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
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	Device->CreateInputLayout(
		inputElements,
		3,
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

	D3D11_BUFFER_DESC lightBufDesc = {};
	lightBufDesc.ByteWidth = sizeof(LightsParams);
	lightBufDesc.Usage = D3D11_USAGE_DEFAULT;
	lightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufDesc.CPUAccessFlags = 0;
	lightBufDesc.MiscFlags = 0;
	lightBufDesc.StructureByteStride = 0;

	Device->CreateBuffer(&lightBufDesc, nullptr, &lightBuffer);

	D3D11_BUFFER_DESC dynamicLightBufDesc = {};
	dynamicLightBufDesc.ByteWidth = sizeof(LightsParams) * 10;
	dynamicLightBufDesc.Usage = D3D11_USAGE_DEFAULT;
	dynamicLightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dynamicLightBufDesc.CPUAccessFlags = 0;
	dynamicLightBufDesc.MiscFlags = 0;
	dynamicLightBufDesc.StructureByteStride = 0;

	res = Device->CreateBuffer(&dynamicLightBufDesc, nullptr, &dynamicLightBuffer);

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

	light.color = Vector4(0.054f, 0.149f, 0.49f, 0);
	light.direction = Vector4(-0.7f, -0.7f, 0, 0);
	light.k = Vector4(0.1f, 100.0f, 1.2f, 0);

	for (int i = 0; i < 10; i++) {
		dynamicLights[i].direction = Vector4(i, 3, 0, 0);
		dynamicLights[i].color = Vector4(1, 1, 0, 0);
		dynamicLights[i].k = Vector4(0, 1.0f, 0.1f, 0);
	}

	for (GameComponent* gameComponent : Components)
	{
		gameComponent->Initialize(vertexShader, pixelShader);
	}
}

HRESULT Game::CompileShaderFromFile(LPCWSTR pFileName, const D3D_SHADER_MACRO* pDefines, LPCSTR pEntryPoint, LPCSTR pShaderModel, ID3DBlob** ppBytecodeBlob)
{
	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;

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
