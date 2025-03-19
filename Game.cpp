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
	Initialize();
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

		cam_rot.y -= Input->MouseOffset.x * 0.01f;
		cam_rot.x -= Input->MouseOffset.y * 0.01f;
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
			auto lookAtPoint = Vector3(0, 0, 0);
			Vector3 camPos = Vector3(distance, 0, 0); // distance - расстояние от камеры
			// до точки просмотра
			Matrix rotMat = Matrix::CreateFromYawPitchRoll(Vector3(0, -cam_rot.y, cam_rot.x));
			camPos = Vector3::Transform(camPos, rotMat) + lookAtPoint; // Финальная позиция камеры
			view_matrix = Matrix::CreateLookAt(camPos, lookAtPoint, Vector3::Transform(Vector3::Up, rotMat));
		}

		Update();
		Draw();
	}

	std::cout << "Hello World!\n";
}

void Game::Update()
{
	for (GameComponent* gameComponent : Components)
	{
		gameComponent->Update();
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

	float color[] = { 0, 0, 0, 0 };
	Context->ClearRenderTargetView(RenderView, color);
	Context->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	Context->OMSetRenderTargets(1, &RenderView, depth_stencil_view_);
	Context->RSSetState(rastState);

	Context->IASetInputLayout(layout);


	for (GameComponent* gameComponent : Components)
	{
		const auto matrix = gameComponent->world_matrix * view_matrix * projection_matrix;
		Context->UpdateSubresource(constantBuffer, 0, nullptr, &matrix, 0, 0);
		Context->VSSetConstantBuffers(0, 1, &constantBuffer);
		Context->PSSetConstantBuffers(0, 1, &constantBuffer);
		gameComponent->Draw();
	}

	Context->OMSetRenderTargets(0, nullptr, nullptr);

	SwapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
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
	res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
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
			MessageBox(Display->hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl", Shader_Macros /*macros*/, nullptr /*include*/, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShaderByteCode, &errorPixelCode);

	Device->CreateVertexShader(
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		nullptr, &vertexShader);

	Device->CreatePixelShader(
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

	Device->CreateInputLayout(
		inputElements,
		3,
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		&layout);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = Device->CreateRasterizerState(&rastDesc, &rastState);

	D3D11_BUFFER_DESC constBufDesc = {};
	constBufDesc.ByteWidth = sizeof(Matrix);
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = 0;
	constBufDesc.MiscFlags = 0;
	constBufDesc.StructureByteStride = 0;

	Device->CreateBuffer(&constBufDesc, nullptr, &constantBuffer);

	for (GameComponent* gameComponent : Components)
	{
		gameComponent->Initialize(vertexShader, pixelShader);
	}
}
