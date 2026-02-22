#include "SkeletalModelComponent.h"
#include "../GameObject.h"
#include "../Transform.h"
#include "../Game.h"

#include <d3dcompiler.h>
#include <stdexcept>

// ============================================================
SkeletalModelComponent::SkeletalModelComponent(
    std::vector<SkeletalMesh>* meshes, Skeleton* skeleton)
    : meshes_(meshes), skeleton_(skeleton)
{
}

SkeletalModelComponent::~SkeletalModelComponent()
{
    if (skinnedVS_) { skinnedVS_->Release();     skinnedVS_ = nullptr; }
    if (skinnedLayout_) { skinnedLayout_->Release(); skinnedLayout_ = nullptr; }
    if (boneBuffer_) { boneBuffer_->Release();    boneBuffer_ = nullptr; }
}

// ============================================================
HRESULT SkeletalModelComponent::CompileShader(
    LPCWSTR file, LPCSTR entry, LPCSTR model, ID3DBlob** blob)
{
    ID3DBlob* err = nullptr;
    HRESULT hr = D3DCompileFromFile(file, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry, model,
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
        0, blob, &err);
    if (FAILED(hr) && err)
    {
        MessageBoxA(nullptr, (char*)err->GetBufferPointer(), "Shader compile error", MB_ICONERROR);
        err->Release();
    }
    return hr;
}

// ============================================================
void SkeletalModelComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* /*context*/)
{
    // --- 1. Skinned vertex shader ---
    ID3DBlob* vsBlob = nullptr;
    HRESULT hr = CompileShader(L"SkinnedVertexShader.hlsl", "main", "vs_5_0", &vsBlob);
    if (FAILED(hr)) return;

    hr = device->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        nullptr, &skinnedVS_);
    if (FAILED(hr)) { vsBlob->Release(); return; }

    // --- 2. Input layout ---
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,     0, offsetof(SKELETAL_VERTEX, X),           D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,     0, offsetof(SKELETAL_VERTEX, NX),          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,        0, offsetof(SKELETAL_VERTEX, texcoord),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,     0, offsetof(SKELETAL_VERTEX, TX),          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,     0, offsetof(SKELETAL_VERTEX, BX),          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT,   0, offsetof(SKELETAL_VERTEX, BoneIndices), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, offsetof(SKELETAL_VERTEX, BoneWeights), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = device->CreateInputLayout(layout, ARRAYSIZE(layout),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        &skinnedLayout_);
    vsBlob->Release();
    if (FAILED(hr)) return;

    // --- 3. Bone palette CB ---
    D3D11_BUFFER_DESC bbd = {};
    bbd.Usage = D3D11_USAGE_DEFAULT;
    bbd.ByteWidth = sizeof(Matrix) * SHADER_MAX_BONES;
    bbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bbd.CPUAccessFlags = 0;
    device->CreateBuffer(&bbd, nullptr, &boneBuffer_);
}

// ============================================================
void SkeletalModelComponent::Draw(ID3D11Device* /*device*/, ID3D11DeviceContext* context)
{
    if (!skinnedVS_ || !skinnedLayout_ || !boneBuffer_) return;

    // --- 1. Bone palette ---
    skeleton_->ComputeGlobalTransforms();
    skeleton_->GetFinalBoneMatrices(cachedBonePalette_);
    cachedBonePalette_.resize(SHADER_MAX_BONES, Matrix::Identity);

    context->UpdateSubresource(boneBuffer_, 0, nullptr, cachedBonePalette_.data(), 0, 0);

    // --- 2. Shader + layout ---
    context->VSSetShader(skinnedVS_, nullptr, 0);
    context->IASetInputLayout(skinnedLayout_);
    context->VSSetConstantBuffers(3, 1, &boneBuffer_);

    // --- 3. Загружаем деформированные вершины в GPU (если есть изменения) ---
    for (auto& mesh : *meshes_)
        mesh.UploadDeformed(context);

    // --- 4. Рисуем ---
    for (auto& mesh : *meshes_)
        mesh.Draw(context);
}

// ============================================================
void SkeletalModelComponent::ApplyDent(const Vector3& hitPosWorld,
    float radius, float depth)
{
    if (!gameObject || cachedBonePalette_.empty()) return;

    // Переводим точку попадания из world-space в model-space Y_Bot'а
    Matrix worldMatrix = gameObject->GetTransform()->GetMatrix();
    Matrix worldInv;
    worldMatrix.Invert(worldInv);

    Vector3 hitPosModel = Vector3::Transform(hitPosWorld, worldInv);

    // Радиус тоже нужно масштабировать в model-space.
    // Y_Bot масштабируется 0.01, значит 1 мировая единица = 100 модельных.
    // Берём обратный масштаб из матрицы (предполагаем uniform scale).
    Vector3 scl = gameObject->GetTransform()->scale;
    float invScale = (scl.x > 1e-6f) ? (1.0f / scl.x) : 1.0f;
    float radiusModel = radius * invScale;
    float depthModel = depth * invScale;

    for (auto& mesh : *meshes_)
        mesh.ApplyDent(hitPosModel, radiusModel, depthModel, cachedBonePalette_);
}