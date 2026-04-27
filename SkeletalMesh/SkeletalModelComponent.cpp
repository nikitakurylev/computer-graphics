#include "SkeletalModelComponent.h"
#include "../GameObject.h"
#include "../Transform.h"
#include "../Game.h"
#include <d3dcompiler.h>
#include "../BenchmarkLogger.h"

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

void SkeletalModelComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext*)
{
    ID3DBlob* vsBlob = nullptr;
    if (FAILED(CompileShader(L"SkinnedVertexShader.hlsl", "main", "vs_5_0", &vsBlob))) return;

    if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        nullptr, &skinnedVS_))) {
        vsBlob->Release(); return;
    }

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(SKELETAL_VERTEX, X),           D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(SKELETAL_VERTEX, NX),          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(SKELETAL_VERTEX, texcoord),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(SKELETAL_VERTEX, TX),          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(SKELETAL_VERTEX, BX),          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT,  0, offsetof(SKELETAL_VERTEX, BoneIndices), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(SKELETAL_VERTEX, BoneWeights), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (FAILED(device->CreateInputLayout(layout, ARRAYSIZE(layout),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &skinnedLayout_)))
    {
        vsBlob->Release(); return;
    }
    vsBlob->Release();

    D3D11_BUFFER_DESC bbd = {};
    bbd.Usage = D3D11_USAGE_DEFAULT;
    bbd.ByteWidth = sizeof(Matrix) * SHADER_MAX_BONES;
    bbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer(&bbd, nullptr, &boneBuffer_);
}

void SkeletalModelComponent::Draw(ID3D11Device*, ID3D11DeviceContext* context)
{
    if (!skinnedVS_ || !skinnedLayout_ || !boneBuffer_) return;

    skeleton_->ComputeGlobalTransforms();

    // Кешируем bone palette для CPU-skinning в ApplyDent
    skeleton_->GetFinalBoneMatrices(cachedBonePalette_);
    cachedBonePalette_.resize(SHADER_MAX_BONES, Matrix::Identity);

    // Кешируем globalTransforms для обратного преобразования смещений
    const int boneCount = (int)skeleton_->bones.size();
    cachedGlobalTransforms_.resize(boneCount);
    for (int i = 0; i < boneCount; ++i)
        cachedGlobalTransforms_[i] = skeleton_->bones[i].globalTransform;

    context->UpdateSubresource(boneBuffer_, 0, nullptr, cachedBonePalette_.data(), 0, 0);
    context->VSSetShader(skinnedVS_, nullptr, 0);
    context->IASetInputLayout(skinnedLayout_);
    context->VSSetConstantBuffers(3, 1, &boneBuffer_);

    for (auto& mesh : *meshes_)
        mesh.UploadToGPU(context);

    for (auto& mesh : *meshes_)
        mesh.Draw(context);
}

void SkeletalModelComponent::ApplyDent(const Vector3& hitPosWorld,
    const Vector3& shotDir,
    float radius, float depth)
{
    // Увеличиваем счётчик ударов, под которым будут идти все замеры
    // от вызовов внутри этого ApplyDent (включая вложенные mesh.ApplyDent).
    int newIdx = BenchmarkLogger::Instance().GetCurrentHitIndex() + 1;
    BenchmarkLogger::Instance().SetCurrentHitIndex(newIdx);

    if (cachedBonePalette_.empty() || cachedGlobalTransforms_.empty()) return;
    if (!gameObject) return;

    // Переводим из world space в model space.
    // bonePalette (offsetMatrix * globalTransform) работает в model space —
    // поэтому hitPos и shotDir тоже нужно туда перевести.
    Matrix worldMatrix = gameObject->GetTransform()->GetMatrix();
    Matrix worldInv;
    worldMatrix.Invert(worldInv);

    Vector3 hitPosModel = Vector3::Transform(hitPosWorld, worldInv);

    // TransformNormal для направления: убираем трансляцию, но оставляем вращение/масштаб.
    // После инверсии матрицы масштаб становится 1/scale = 100 для Y_Bot.
    // Нормализуем — длина не важна, важно направление.
    Vector3 shotDirModel = Vector3::TransformNormal(shotDir, worldInv);
    shotDirModel.Normalize();

    // Масштабируем радиус и глубину из world units в model units.
    // Y_Bot: scale = 0.01, значит 1 world unit = 100 model units.
    float invScale = 1.0f / gameObject->GetTransform()->scale.x;
    float radiusModel = radius * invScale;
    float depthModel = depth * invScale;

    for (auto& mesh : *meshes_)
        mesh.ApplyDent(hitPosModel, shotDirModel,
            radiusModel, depthModel,
            cachedBonePalette_,
            cachedGlobalTransforms_);
}