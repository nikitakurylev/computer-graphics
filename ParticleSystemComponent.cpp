#include "ParticleSystemComponent.h"
#include "Game.h"
#include <d3dcompiler.h>
#include <iostream>
#include <WICTextureLoader.h>
#include <random>

using namespace DirectX::SimpleMath;

using int4 = struct SortConstants
{
    int x, y, z, w;
};

int align(int value, int alignment)
{
    return (value + (alignment - 1)) & ~(alignment - 1);
}

ParticleSystemComponent::ParticleSystemComponent()
{
}

void ParticleSystemComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    emitterProps_ = {
        {0.0, 0.0, 0.0},
        256,
        15.0f
    };

    {
        ID3DBlob* errorBlob = nullptr;
        HRESULT res = D3DCompileFromFile(L"./Shaders/particles/Emit.hlsl",
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "CSMain",
            "cs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
            0,
            &emitBlobCs_,
            &errorBlob);

        if (FAILED(res)) {
            if (errorBlob) {
                char* compileErrors = (char*)(errorBlob->GetBufferPointer());

                std::cout << compileErrors << std::endl;

                assert(false);
            }
        }


        device->CreateComputeShader(
            emitBlobCs_->GetBufferPointer(),
            emitBlobCs_->GetBufferSize(),
            nullptr, &emitCs_);
    }

    {
        ID3DBlob* errorBlob = nullptr;
        HRESULT res = D3DCompileFromFile(L"./Shaders/particles/Simulate.hlsl",
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "CSMain",
            "cs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
            0,
            &simulateBlobCs_,
            &errorBlob);

        if (FAILED(res)) {
            if (errorBlob) {
                char* compileErrors = (char*)(errorBlob->GetBufferPointer());

                std::cout << compileErrors << std::endl;

                assert(false);
            }
        }


        device->CreateComputeShader(
            simulateBlobCs_->GetBufferPointer(),
            simulateBlobCs_->GetBufferSize(),
            nullptr, &simulateCs_);
    }

    {
        ID3DBlob* errorBlob = nullptr;
        HRESULT res = D3DCompileFromFile(L"./Shaders/particles/Reset.hlsl",
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "CSMain",
            "cs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
            0,
            &resetBlboCs_,
            &errorBlob);

        if (FAILED(res)) {
            if (errorBlob) {
                char* compileErrors = (char*)(errorBlob->GetBufferPointer());

                std::cout << compileErrors << std::endl;

                assert(false);
            }
        }


        device->CreateComputeShader(
            resetBlboCs_->GetBufferPointer(),
            resetBlboCs_->GetBufferSize(),
            nullptr, &resetCs_);
    }

    {
        D3DCompileFromFile(
            L"./Shaders/particles/Render.hlsl",
            nullptr,
            nullptr,
            "VSMain",
            "vs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
            0,
            &vertexBlob_,
            nullptr
        );

        ID3DBlob* errorBlob = nullptr;
        D3DCompileFromFile(
            L"./Shaders/particles/Render.hlsl",
            nullptr,
            nullptr,
            "GSMain",
            "gs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
            0,
            &geometryBlob_,
            nullptr
        );
        
        device->CreateVertexShader(
            vertexBlob_->GetBufferPointer(),
            vertexBlob_->GetBufferSize(),
            nullptr, &vertexShader_
        );

        device->CreateGeometryShader(
            geometryBlob_->GetBufferPointer(),
            geometryBlob_->GetBufferSize(),
            nullptr, &geometryShader_
        );
    }

    {
        D3D11_BLEND_DESC blendDesc{};
        blendDesc.AlphaToCoverageEnable = false;
        blendDesc.IndependentBlendEnable = false;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[0].BlendEnable = true;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        device->CreateBlendState(&blendDesc, &blendState_);
    }

    {
        D3D11_BUFFER_DESC constBufDesc = {};
        constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
        constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constBufDesc.MiscFlags = 0;
        constBufDesc.StructureByteStride = 0;
        constBufDesc.ByteWidth = sizeof(EmitterProperties);

        device->CreateBuffer(&constBufDesc, nullptr, &emitterBuffer_);
    }

    {
        D3D11_BUFFER_DESC constBufDesc = {};
        constBufDesc.Usage = D3D11_USAGE_DYNAMIC;
        constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constBufDesc.MiscFlags = 0;
        constBufDesc.StructureByteStride = 0;
        constBufDesc.ByteWidth = sizeof(DirectX::SimpleMath::Vector4);

        device->CreateBuffer(&constBufDesc, nullptr, &frameTimeBuffer_);
    }

    {
        auto res = DirectX::CreateWICTextureFromFile(
            device,
            L"./whiteCircle.png",
            &albedoTexture_,
            &albedoSrv_);
    }

    {
        const int WidthHeight = 1024;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distrib(-1, 1);

        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = WidthHeight;
        desc.Height = WidthHeight;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        std::vector<float> values(WidthHeight* WidthHeight * 4);
        float* ptr = values.data();
        for (UINT i = 0; i < desc.Width * desc.Height; i++)
        {
            ptr[0] = distrib(gen);
            ptr[1] = distrib(gen);
            ptr[2] = distrib(gen);
            ptr[3] = distrib(gen);
            ptr += 4;
        }

        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = values.data();
        data.SysMemPitch = desc.Width * 16;
        data.SysMemSlicePitch = 0;

        device->CreateTexture2D(&desc, &data, (ID3D11Texture2D**)&randomTexture_);

        D3D11_SHADER_RESOURCE_VIEW_DESC srv;
        srv.Format = desc.Format;
        srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv.Texture2D.MipLevels = 1;
        srv.Texture2D.MostDetailedMip = 0;

        device->CreateShaderResourceView(randomTexture_, &srv, &randomSrv_);
    }

    {
        D3D11_BUFFER_DESC cbDesc;
        ZeroMemory(&cbDesc, sizeof(cbDesc));
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.ByteWidth = sizeof(int4);
        device->CreateBuffer(&cbDesc, nullptr, &dispatchInfoBuffer_);
    }

    {
        ID3DBlob* pBlob;
        ID3DBlob* pErrorBlob;

        // Step sort shader
        HRESULT hr = D3DCompileFromFile(L"./Shaders/particles/SortStep2.hlsl", nullptr, nullptr, "BitonicSortStep",
            "cs_5_0", 0, 0, &pBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob != nullptr)
                OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        }

        device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &sortStep_);
    }

    {
        ID3DBlob* pBlob;
        ID3DBlob* pErrorBlob;

        const D3D10_SHADER_MACRO innerDefines[2] = { {"SORT_SIZE", "512"}, {nullptr, 0} };
        HRESULT hr = D3DCompileFromFile(L"./Shaders/particles/SortInner.hlsl", innerDefines, nullptr, "BitonicInnerSort",
            "cs_5_0", 0, 0, &pBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob != NULL)
                OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        }
        device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &sortInner512_);
    }

    {
        ID3DBlob* pBlob;
        ID3DBlob* pErrorBlob;

        const D3D10_SHADER_MACRO cs512Defines[2] = { {"SORT_SIZE", "512"}, {nullptr, 0} };
        HRESULT hr = D3DCompileFromFile(L"./Shaders/particles/Sort512.hlsl", cs512Defines, nullptr, "BitonicSortLDS", "cs_5_0", 0,
            0, &pBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob != nullptr)
                OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        }
        device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &sort512_);
    }

    {
        ID3DBlob* pBlob;
        ID3DBlob* pErrorBlob;

        const D3D10_SHADER_MACRO cs512Defines[2] = { {"SORT_SIZE", "512"}, {nullptr, 0} };
        HRESULT  hr = D3DCompileFromFile(L"./Shaders/particles/InitSort.hlsl", nullptr, nullptr, "InitDispatchArgs",
            "cs_5_0", 0, 0, &pBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob != nullptr)
                OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        }
        device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &sortInitArgs_);
    }

    {
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = MAX_PARTICLE_COUNT * sizeof(Particle);
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = sizeof(Particle);

        device->CreateBuffer(&bufferDesc, nullptr, &particleBuffer_);

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Format = DXGI_FORMAT_UNKNOWN;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srv_desc.Buffer.ElementOffset = 0;
        srv_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;

        device->CreateShaderResourceView(particleBuffer_, &srv_desc, &particleBufferSrv_);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.Flags = 0;
        uav_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;
        device->CreateUnorderedAccessView(particleBuffer_, &uav_desc, &particleBufferUav_);
    }

    {
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = MAX_PARTICLE_COUNT * sizeof(uint32_t);
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = sizeof(uint32_t);

        std::vector<uint32_t> indices;
        for (uint32_t i = 0; i < MAX_PARTICLE_COUNT; ++i)
        {
            indices.push_back(i);
        }
        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = indices.data();

        device->CreateBuffer(&bufferDesc, nullptr, &deadBuffer_);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
        uav_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;
        device->CreateUnorderedAccessView(deadBuffer_, &uav_desc, &deadBufferUav_);
    }

    {
        struct ParticleIndexElement
        {
            float distance;
            float index;
        };

        D3D11_BUFFER_DESC aliveIndexBufferDesc{};
        aliveIndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        aliveIndexBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        aliveIndexBufferDesc.CPUAccessFlags = 0;
        aliveIndexBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        aliveIndexBufferDesc.ByteWidth = sizeof(ParticleIndexElement) * MAX_PARTICLE_COUNT;
        aliveIndexBufferDesc.StructureByteStride = sizeof(ParticleIndexElement);

        device->CreateBuffer(&aliveIndexBufferDesc, nullptr, &aliveBuffer_);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
        uav_desc.Buffer.NumElements = MAX_PARTICLE_COUNT;
        device->CreateUnorderedAccessView(aliveBuffer_, &uav_desc, &aliveBufferUav_);

        D3D11_SHADER_RESOURCE_VIEW_DESC aliveIndexSRVDesc{};
        aliveIndexSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
        aliveIndexSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        aliveIndexSRVDesc.Buffer.ElementOffset = 0;
        aliveIndexSRVDesc.Buffer.NumElements = MAX_PARTICLE_COUNT;

        device->CreateShaderResourceView(aliveBuffer_, &aliveIndexSRVDesc, &aliveBufferSrv_);
    }

    {
        struct DeadListCountConstantBuffer
        {
            UINT nbDeadParticles;

            UINT padding[3];
        };

        D3D11_BUFFER_DESC desc{};  
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;
        desc.ByteWidth = sizeof(DeadListCountConstantBuffer);

        device->CreateBuffer(&desc, nullptr, &deadCounterBuffer_);
        device->CreateBuffer(&desc, nullptr, &aliveCounterBuffer_);
    }

    {
        D3D11_BUFFER_DESC indirectDrawArgsBuffer{};
        indirectDrawArgsBuffer.Usage = D3D11_USAGE_DEFAULT;
        indirectDrawArgsBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        indirectDrawArgsBuffer.ByteWidth = 5 * sizeof(UINT);
        indirectDrawArgsBuffer.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

        device->CreateBuffer(&indirectDrawArgsBuffer, nullptr, &indirectDrawBuffer_);

        D3D11_UNORDERED_ACCESS_VIEW_DESC indirectDrawArgsUAVDesc{};
        indirectDrawArgsUAVDesc.Format = DXGI_FORMAT_R32_UINT;
        indirectDrawArgsUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        indirectDrawArgsUAVDesc.Buffer.FirstElement = 0;
        indirectDrawArgsUAVDesc.Buffer.NumElements = 5;
        indirectDrawArgsUAVDesc.Buffer.Flags = 0;

        device->CreateUnorderedAccessView(indirectDrawBuffer_, &indirectDrawArgsUAVDesc, &indirectDrawBufferUav_);
    }

    {
        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        desc.ByteWidth = 4 * sizeof(UINT);
        desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
        device->CreateBuffer(&desc, nullptr, &indirectSortArgsBuffer_);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav{};
        uav.Format = DXGI_FORMAT_R32_UINT;
        uav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav.Buffer.FirstElement = 0;
        uav.Buffer.NumElements = 4;
        uav.Buffer.Flags = 0;
        device->CreateUnorderedAccessView(indirectSortArgsBuffer_, &uav, &indirectSortArgsBufferUav_);
    }

    Reload(context);
}

void ParticleSystemComponent::Update(float deltaTime)
{
    _deltaTime = deltaTime;
}

void ParticleSystemComponent::Draw(ID3D11Device* device, ID3D11DeviceContext* context)
{
    context->VSSetShader(vertexShader_, nullptr, 0);
    context->GSSetShader(geometryShader_, nullptr, 0);

    ID3D11Buffer* nullVertexBuffer = nullptr;
    UINT stride = 0;
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &nullVertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    //context->OMSetBlendState(blendState_, nullptr, 0xffffffff);

    context->VSSetConstantBuffers(3, 1, &aliveCounterBuffer_);

    context->VSSetShaderResources(0, 1, &particleBufferSrv_);
    context->VSSetShaderResources(1, 1, &aliveBufferSrv_);

    context->PSSetShaderResources(0, 1, &albedoSrv_);

    context->DrawInstancedIndirect(indirectDrawBuffer_, 0);
    //context->Draw(MAX_PARTICLE_COUNT, 0);

    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    context->GSSetShader(nullptr, nullptr, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ParticleSystemComponent::Compute(ID3D11DeviceContext* context)
{
    Simulate(context);

    Sort(context);
}

void ParticleSystemComponent::Reload(ID3D11DeviceContext* context)
{
    UINT initialCount[] = { 0 };
    context->CSSetUnorderedAccessViews(0, 1, &deadBufferUav_, initialCount);
    context->CSSetUnorderedAccessViews(1, 1, &particleBufferUav_, initialCount);
    int numThreadGroups = align(emitterProps_.maxNumToEmit, 1024) / 1024;
    context->CSSetShader(resetCs_, nullptr, 0);
    context->Dispatch(numThreadGroups, 1, 1);
}

void ParticleSystemComponent::DestroyResources()
{
}

void ParticleSystemComponent::Sort(ID3D11DeviceContext* context)
{
    ID3D11UnorderedAccessView* prevUAV = nullptr;
    context->CSGetUnorderedAccessViews(0, 1, &prevUAV);

    ID3D11Buffer* prevCBs[] = { nullptr, nullptr };
    context->CSGetConstantBuffers(0, ARRAYSIZE(prevCBs), prevCBs);

    ID3D11Buffer* cbs[] = { aliveCounterBuffer_, dispatchInfoBuffer_ };
    context->CSSetConstantBuffers(0, ARRAYSIZE(cbs), cbs);

    context->CSSetUnorderedAccessViews(0, 1, &indirectSortArgsBufferUav_, nullptr);

    context->CSSetShader(sortInitArgs_, nullptr, 0);
    context->Dispatch(1, 1, 1);

    context->CSSetUnorderedAccessViews(0, 1, &aliveBufferUav_, nullptr);

    bool bDone = SortInitial(context, MAX_PARTICLE_COUNT);

    int presorted = 512;
    while (!bDone)
    {
        bDone = SortIncremental(context, presorted, MAX_PARTICLE_COUNT);
        presorted *= 2;
    }

    context->CSSetUnorderedAccessViews(0, 1, &prevUAV, nullptr);
    context->CSSetConstantBuffers(0, ARRAYSIZE(prevCBs), prevCBs);

    if (prevUAV)
        prevUAV->Release();

    for (size_t i = 0; i < ARRAYSIZE(prevCBs); i++)
        if (prevCBs[i])
            prevCBs[i]->Release();
}

void ParticleSystemComponent::Emit(ID3D11DeviceContext* context)
{
    ID3D11UnorderedAccessView* uavs[] = { particleBufferUav_, deadBufferUav_ };
    UINT initialCounts[] = { (UINT)-1, (UINT)-1 };
    context->CSSetUnorderedAccessViews(0, std::size(uavs), uavs, initialCounts);

    ID3D11Buffer* buffers[] = { frameTimeBuffer_, emitterBuffer_, deadCounterBuffer_ };
    auto time = DirectX::SimpleMath::Vector4(_deltaTime, 0.0, 0.0, 0.0);

    UpdateBuffer(context, frameTimeBuffer_, &time, sizeof(DirectX::SimpleMath::Vector4));
    UpdateBuffer(context, emitterBuffer_, &emitterProps_, sizeof(EmitterProperties));
    context->CSSetConstantBuffers(1, ARRAYSIZE(buffers), buffers);

    ID3D11ShaderResourceView* srvs[] = { randomSrv_ };
    context->CSSetShaderResources(0, 1, srvs);

    context->CSSetShader(emitCs_, nullptr, 0);

    context->CopyStructureCount(deadCounterBuffer_, 0, deadBufferUav_);

    int numThreadGroups = align(emitterProps_.maxNumToEmit, X_NUMTHREADS) / X_NUMTHREADS;
    context->Dispatch(numThreadGroups, 1, 1);
}

void ParticleSystemComponent::Simulate(ID3D11DeviceContext* context)
{
    context->CopyStructureCount(aliveCounterBuffer_, 0, aliveBufferUav_);
    UINT initialCount[] = { (UINT)-1 };
    UINT aliveInitialCount[] = { 0 };

    context->CSSetUnorderedAccessViews(0, 1, &particleBufferUav_, initialCount);
    context->CSSetUnorderedAccessViews(1, 1, &deadBufferUav_, initialCount);
    context->CSSetUnorderedAccessViews(2, 1, &aliveBufferUav_, aliveInitialCount);
    context->CSSetUnorderedAccessViews(3, 1, &indirectDrawBufferUav_, initialCount);

    context->CSSetShader(simulateCs_, nullptr, 0);
    context->Dispatch(align(MAX_PARTICLE_COUNT, 256) / 256, 1, 1);

    ID3D11UnorderedAccessView* const ruav[1] = { nullptr };
    context->CSSetUnorderedAccessViews(0, 1, ruav, nullptr);
    context->CSSetUnorderedAccessViews(1, 1, ruav, nullptr);
    context->CSSetUnorderedAccessViews(2, 1, ruav, nullptr);
    context->CSSetUnorderedAccessViews(3, 1, ruav, nullptr);
}

void ParticleSystemComponent::UpdateBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, void* data, size_t size) {
    D3D11_MAPPED_SUBRESOURCE res = {};
    context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    memcpy(res.pData, data, size);
    context->Unmap(buffer, 0);
}

bool ParticleSystemComponent::SortInitial(ID3D11DeviceContext* context, unsigned int maxSize)
{
    bool bDone = true;

    unsigned int numThreadGroups = ((maxSize - 1) >> 9) + 1;

    if (numThreadGroups > 1) bDone = false;

    context->CSSetShader(sort512_, nullptr, 0);
    context->DispatchIndirect(indirectSortArgsBuffer_, 0);

    return bDone;
}

bool ParticleSystemComponent::SortIncremental(ID3D11DeviceContext* context, unsigned int presorted, unsigned int maxSize)
{
    bool bDone = true;
    context->CSSetShader(sortStep_, nullptr, 0);

    unsigned int numThreadGroups = 0;

    if (maxSize > presorted)
    {
        if (maxSize > presorted * 2)
            bDone = false;

        unsigned int pow2 = presorted;
        while (pow2 < maxSize)
            pow2 *= 2;
        numThreadGroups = pow2 >> 9;
    }

    unsigned int nMergeSize = presorted * 2;
    for (unsigned int nMergeSubSize = nMergeSize >> 1; nMergeSubSize > 256; nMergeSubSize = nMergeSubSize >> 1)
    {
        D3D11_MAPPED_SUBRESOURCE MappedResource;

        context->Map(dispatchInfoBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
        auto sc = (SortConstants*)MappedResource.pData;
        sc->x = nMergeSubSize;
        if (nMergeSubSize == nMergeSize >> 1)
        {
            sc->y = (2 * nMergeSubSize - 1);
            sc->z = -1;
        }
        else
        {
            sc->y = nMergeSubSize;
            sc->z = 1;
        }
        sc->w = 0;
        context->Unmap(dispatchInfoBuffer_, 0);

        context->Dispatch(numThreadGroups, 1, 1);
    }

    context->CSSetShader(sortInner512_, nullptr, 0);
    context->Dispatch(numThreadGroups, 1, 1);

    return bDone;
}
