#include "PlaneComponent.h"
#include "GameObject.h"
#include "Transform.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// ============================================================
PlaneComponent::PlaneComponent(float size, float tiling)
    : size_(size), tiling_(tiling)
{
}

PlaneComponent::~PlaneComponent()
{
    if (mesh_) { mesh_->Close(); delete mesh_; mesh_ = nullptr; }
    if (defaultWhite_) { defaultWhite_->Release();  defaultWhite_ = nullptr; }
    if (defaultNormal_) { defaultNormal_->Release(); defaultNormal_ = nullptr; }
}

// ============================================================
void PlaneComponent::CreateDefaultTextures(ID3D11Device* device)
{
    // 1x1 белая текстура
    {
        const UINT white = 0xFFFFFFFF;
        D3D11_TEXTURE2D_DESC d = {};
        d.Width = d.Height = 1; d.MipLevels = d.ArraySize = 1;
        d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        d.SampleDesc.Count = 1;
        d.Usage = D3D11_USAGE_IMMUTABLE;
        d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA sd = {}; sd.pSysMem = &white; sd.SysMemPitch = 4;
        ID3D11Texture2D* t = nullptr;
        device->CreateTexture2D(&d, &sd, &t);
        if (t) { device->CreateShaderResourceView(t, nullptr, &defaultWhite_); t->Release(); }
    }
    // 1x1 нейтральная нормаль (128,128,255,255)
    {
        const unsigned char n[4] = { 128, 128, 255, 255 };
        D3D11_TEXTURE2D_DESC d = {};
        d.Width = d.Height = 1; d.MipLevels = d.ArraySize = 1;
        d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        d.SampleDesc.Count = 1;
        d.Usage = D3D11_USAGE_IMMUTABLE;
        d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA sd = {}; sd.pSysMem = n; sd.SysMemPitch = 4;
        ID3D11Texture2D* t = nullptr;
        device->CreateTexture2D(&d, &sd, &t);
        if (t) { device->CreateShaderResourceView(t, nullptr, &defaultNormal_); t->Release(); }
    }
}

// ============================================================
static VERTEX MakeVtx(float x, float z, float u, float v)
{
    VERTEX vtx = {};
    vtx.X = x;    vtx.Y = 0.0f; vtx.Z = z;
    vtx.NX = 0.0f; vtx.NY = 1.0f; vtx.NZ = 0.0f;
    vtx.texcoord = XMFLOAT2(u, v);
    vtx.TX = 1.0f; vtx.TY = 0.0f; vtx.TZ = 0.0f;
    vtx.BX = 0.0f; vtx.BY = 0.0f; vtx.BZ = -1.0f;
    return vtx;
}

void PlaneComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* /*context*/)
{
    CreateDefaultTextures(device);

    const float h = size_ * 0.5f;
    const float t = tiling_;

    std::vector<VERTEX> verts;
    verts.push_back(MakeVtx(-h, h, 0, 0)); // top-left
    verts.push_back(MakeVtx(h, h, t, 0)); // top-right
    verts.push_back(MakeVtx(h, -h, t, t)); // bot-right
    verts.push_back(MakeVtx(-h, -h, 0, t)); // bot-left

    std::vector<UINT> indices = { 0, 1, 2,  0, 2, 3 };

    Material mat = {};
    mat.albedo = defaultWhite_;
    mat.ao = defaultWhite_;
    mat.metallic = defaultWhite_;
    mat.roughness = defaultWhite_;
    mat.normal = defaultNormal_;
    mat.baseColorFactor = XMFLOAT4(0.55f, 0.55f, 0.55f, 1.0f);
    mat.materialParams = XMFLOAT4(0.0f, 0.85f, 1.0f, 0.0f);

    mesh_ = new Mesh(device, verts, indices, {}, mat);
}

// ============================================================
void PlaneComponent::Draw(ID3D11Device* /*device*/, ID3D11DeviceContext* context)
{
    if (mesh_) mesh_->Draw(context);
}

// ============================================================
bool PlaneComponent::GetGlobalAABB(BoundingBox& outBox)
{
    if (!mesh_) return false;
    if (gameObject)
    {
        Matrix world = gameObject->GetTransform()->GetMatrix();
        mesh_->aabb.Transform(outBox, world);
        return true;
    }
    outBox = mesh_->aabb;
    return true;
}