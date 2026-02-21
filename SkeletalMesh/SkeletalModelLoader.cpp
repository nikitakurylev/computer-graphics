#include "SkeletalModelLoader.h"

#include <Windows.h>
#include "../SimpleTexturedDirectx11/TextureLoader.h"   // проектный WIC — одна сигнатура, нет амбигвозности
#include <d3d11.h>
#include <cstdint>

using namespace DirectX;
using namespace DirectX::SimpleMath;

// ============================================================
SkeletalModelData* SkeletalModelLoader::Load(const std::string& filename)
{
    Assimp::Importer importer;

    // Устанавливаем рабочую директорию туда, где лежит exe
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring wbuf(buffer);
    auto pos = wbuf.find_last_of(L"\\/");
    SetCurrentDirectory(wbuf.substr(0, pos).c_str());

    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_ConvertToLeftHanded |
        aiProcess_CalcTangentSpace |
        aiProcess_LimitBoneWeights |   // обрезает до 4 весов на вершину
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        MessageBoxA(hwnd_, importer.GetErrorString(), "SkeletalModelLoader Error", MB_ICONERROR);
        return nullptr;
    }

    auto lastSlash = filename.find_last_of("/\\");
    directory_ = (lastSlash != std::string::npos) ? filename.substr(0, lastSlash) : "";

    auto* out = new SkeletalModelData();

    // --- 1. Собираем все offset-матрицы костей из мешей ---
    boneOffsets_cache_.clear();
    CollectBoneNames(scene, boneOffsets_cache_);

    // --- 2. Строим иерархию скелета из узлов сцены ---
    ExtractBoneHierarchy(scene->mRootNode, scene, out->skeleton, -1, Matrix::Identity);

    // Заполняем offset-матрицы
    for (auto& bone : out->skeleton.bones)
    {
        auto it = boneOffsets_cache_.find(bone.name);
        if (it != boneOffsets_cache_.end())
            bone.offsetMatrix = it->second;
        else
            bone.offsetMatrix = Matrix::Identity;
    }

    // Вычисляем глобальные трансформы (T-pose)
    out->skeleton.ComputeGlobalTransforms();

    // --- 3. Обрабатываем меши ---
    ProcessNode(scene->mRootNode, scene, out, Matrix::Identity);

    // --- 4. Создаём GPU-буферы ---
    for (auto& mesh : out->meshes)
        mesh.SetupGPU(dev_);

    out->valid = true;
    return out;
}

// ============================================================
void SkeletalModelLoader::CollectBoneNames(
    const aiScene* scene,
    std::unordered_map<std::string, Matrix>& boneOffsets)
{
    for (UINT m = 0; m < scene->mNumMeshes; ++m)
    {
        aiMesh* mesh = scene->mMeshes[m];
        for (UINT b = 0; b < mesh->mNumBones; ++b)
        {
            aiBone* bone = mesh->mBones[b];
            std::string name = bone->mName.C_Str();
            // Берём offset-матрицу только один раз (первый меш где встретили кость)
            if (boneOffsets.find(name) == boneOffsets.end())
                boneOffsets[name] = ToMatrix(bone->mOffsetMatrix);
        }
    }
}

// ============================================================
//  Вспомогательная: проверяет, есть ли в поддереве узла хотя бы одна кость
// ============================================================
static bool SubtreeHasBone(aiNode* node,
    const std::unordered_map<std::string, Matrix>& boneOffsets)
{
    if (boneOffsets.count(node->mName.C_Str()))
        return true;
    for (UINT i = 0; i < node->mNumChildren; ++i)
        if (SubtreeHasBone(node->mChildren[i], boneOffsets))
            return true;
    return false;
}

// ============================================================
//  Строит иерархию костей:
//  - Добавляем узел если он сам кость (есть в boneOffsets)
//    ИЛИ если в его поддереве есть кости (нужен как промежуточный родитель)
//  - Защита от дублей: если имя уже в skeleton — пропускаем
// ============================================================
void SkeletalModelLoader::ExtractBoneHierarchy(
    aiNode* node, const aiScene* scene,
    Skeleton& skeleton, int parentIndex,
    const Matrix& /*unused*/)
{
    std::string name = node->mName.C_Str();

    // Пропускаем узлы у которых нет костей ни в себе ни в потомках
    if (!boneOffsets_cache_.count(name) && !SubtreeHasBone(node, boneOffsets_cache_))
    {
        // Всё равно обходим детей с тем же parentIndex
        for (UINT i = 0; i < node->mNumChildren; ++i)
            ExtractBoneHierarchy(node->mChildren[i], scene, skeleton, parentIndex, Matrix::Identity);
        return;
    }

    int currentIndex = parentIndex;

    // Добавляем узел только если его имя ещё не в скелете
    if (skeleton.boneNameToIndex.find(name) == skeleton.boneNameToIndex.end())
    {
        aiVector3D pos, scale;
        aiQuaternion rot;
        node->mTransformation.Decompose(scale, rot, pos);

        Bone bone;
        bone.name = name;
        bone.parentIndex = parentIndex;
        bone.localTransform =
            Matrix::CreateScale(scale.x, scale.y, scale.z) *
            Matrix::CreateFromQuaternion(Quaternion(rot.x, rot.y, rot.z, rot.w)) *
            Matrix::CreateTranslation(pos.x, pos.y, pos.z);
        bone.offsetMatrix = Matrix::Identity; // заполняется позже из boneOffsets

        currentIndex = (int)skeleton.bones.size();
        skeleton.boneNameToIndex[name] = currentIndex;
        skeleton.bones.push_back(bone);
    }
    else
    {
        // Дубль — просто берём уже существующий индекс
        currentIndex = skeleton.boneNameToIndex[name];
    }

    for (UINT i = 0; i < node->mNumChildren; ++i)
        ExtractBoneHierarchy(node->mChildren[i], scene, skeleton, currentIndex, Matrix::Identity);
}

// ============================================================
void SkeletalModelLoader::ProcessNode(
    aiNode* node, const aiScene* scene,
    SkeletalModelData* out, const Matrix& /*parentTransform*/)
{
    for (UINT i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        out->meshes.push_back(ProcessMesh(mesh, scene, out));
    }
    for (UINT i = 0; i < node->mNumChildren; ++i)
        ProcessNode(node->mChildren[i], scene, out, Matrix::Identity);
}

// ============================================================
SkeletalMesh SkeletalModelLoader::ProcessMesh(
    aiMesh* mesh, const aiScene* scene,
    SkeletalModelData* out)
{
    SkeletalMesh sm;

    // --- Вершины ---
    sm.vertices.resize(mesh->mNumVertices);
    for (UINT i = 0; i < mesh->mNumVertices; ++i)
    {
        auto& v = sm.vertices[i];
        v.X = mesh->mVertices[i].x;
        v.Y = mesh->mVertices[i].y;
        v.Z = mesh->mVertices[i].z;

        if (mesh->HasNormals())
        {
            v.NX = mesh->mNormals[i].x; v.NY = mesh->mNormals[i].y; v.NZ = mesh->mNormals[i].z;
        }

        if (mesh->mTextureCoords[0])
        {
            v.texcoord.x = mesh->mTextureCoords[0][i].x; v.texcoord.y = mesh->mTextureCoords[0][i].y;
        }

        if (mesh->HasTangentsAndBitangents())
        {
            v.TX = mesh->mTangents[i].x;   v.TY = mesh->mTangents[i].y;   v.TZ = mesh->mTangents[i].z;
            v.BX = mesh->mBitangents[i].x; v.BY = mesh->mBitangents[i].y; v.BZ = mesh->mBitangents[i].z;
        }
    }

    // --- Индексы ---
    for (UINT i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace& face = mesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; ++j)
            sm.indices.push_back(face.mIndices[j]);
    }

    // --- Skin weights (кости) ---
    //  Для каждой вершины держим счётчик заполненных слотов
    std::vector<int> slotCount(mesh->mNumVertices, 0);

    for (UINT b = 0; b < mesh->mNumBones; ++b)
    {
        aiBone* bone = mesh->mBones[b];
        std::string bn = bone->mName.C_Str();
        int boneIdx = out->skeleton.GetBoneIndex(bn);

        if (boneIdx < 0) continue; // кость не в скелете — пропускаем

        for (UINT w = 0; w < bone->mNumWeights; ++w)
        {
            UINT   vi = bone->mWeights[w].mVertexId;
            float  weight = bone->mWeights[w].mWeight;
            int    slot = slotCount[vi];

            if (slot >= MAX_BONE_INFLUENCE) continue; // все слоты заняты

            sm.vertices[vi].BoneIndices[slot] = boneIdx;
            sm.vertices[vi].BoneWeights[slot] = weight;
            slotCount[vi]++;
        }
    }

    // --- Материал ---
    if (mesh->mMaterialIndex >= 0)
        sm.material = LoadMaterial(scene->mMaterials[mesh->mMaterialIndex], scene);
    else
    {
        sm.material.albedo = GetDefaultWhite();
        sm.material.ao = GetDefaultWhite();
        sm.material.metallic = GetDefaultWhite();
        sm.material.roughness = GetDefaultWhite();
        sm.material.normal = GetDefaultNormal();
        sm.material.baseColorFactor = XMFLOAT4(1, 1, 1, 1);
        sm.material.materialParams = XMFLOAT4(0, 1, 1, 0);
    }

    return sm;
}

// ============================================================
Material SkeletalModelLoader::LoadMaterial(aiMaterial* mat, const aiScene* scene)
{
    Material m = {};
    m.albedo = GetDefaultWhite();
    m.ao = GetDefaultWhite();
    m.metallic = GetDefaultWhite();
    m.roughness = GetDefaultWhite();
    m.normal = GetDefaultNormal();
    m.baseColorFactor = XMFLOAT4(1, 1, 1, 1);
    m.materialParams = XMFLOAT4(0, 1, 1, 0);

    if (!mat) return m;

    // Base color
    aiColor4D col;
    if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_BASE_COLOR, &col) ||
        AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &col))
        m.baseColorFactor = XMFLOAT4(col.r, col.g, col.b, col.a);

    float metallic = 0.f, roughness = 1.f;
    aiGetMaterialFloat(mat, AI_MATKEY_METALLIC_FACTOR, &metallic);
    aiGetMaterialFloat(mat, AI_MATKEY_ROUGHNESS_FACTOR, &roughness);
    m.materialParams.x = metallic;
    m.materialParams.y = roughness;

    // Albedo
    Texture albedo = LoadTexture(mat, aiTextureType_BASE_COLOR, "albedo", scene);
    if (albedo.path.empty()) albedo = LoadTexture(mat, aiTextureType_DIFFUSE, "albedo", scene);
    if (albedo.texture) m.albedo = albedo.texture;

    // Normal
    Texture normal = LoadTexture(mat, aiTextureType_NORMALS, "normal", scene);
    if (normal.path.empty()) normal = LoadTexture(mat, aiTextureType_HEIGHT, "normal", scene);
    if (normal.texture) m.normal = normal.texture;

    // Metallic / Roughness
    Texture metalTex = LoadTexture(mat, aiTextureType_METALNESS, "metallic", scene);
    if (metalTex.texture) m.metallic = metalTex.texture;

    Texture roughTex = LoadTexture(mat, aiTextureType_DIFFUSE_ROUGHNESS, "roughness", scene);
    if (roughTex.texture) m.roughness = roughTex.texture;

    // AO
    Texture aoTex = LoadTexture(mat, aiTextureType_AMBIENT_OCCLUSION, "ao", scene);
    if (aoTex.path.empty()) aoTex = LoadTexture(mat, aiTextureType_LIGHTMAP, "ao", scene);
    if (aoTex.texture) m.ao = aoTex.texture;

    return m;
}

// ============================================================
Texture SkeletalModelLoader::LoadTexture(
    aiMaterial* mat, aiTextureType type, const char* typeName, const aiScene* scene)
{
    Texture tex = {};
    if (!mat || mat->GetTextureCount(type) == 0) return tex;

    aiString str;
    if (mat->GetTexture(type, 0, &str) != AI_SUCCESS) return tex;

    // Embedded?
    const aiTexture* embedded = scene->GetEmbeddedTexture(str.C_Str());
    if (embedded)
        return LoadEmbeddedTexture(embedded, typeName);

    std::string path = ResolveTexturePath(str);
    // Проверяем кэш
    for (auto& t : textures_loaded_)
        if (t.path == path) return t;

    std::wstring wpath(path.begin(), path.end());
    HRESULT hr = CreateWICTextureFromFile(dev_, ctx_, wpath.c_str(),
        nullptr, &tex.texture);
    if (FAILED(hr))
    {
        tex.texture = GetDefaultWhite();
        return tex;
    }

    tex.type = typeName;
    tex.path = path;
    textures_loaded_.push_back(tex);
    return tex;
}

// ============================================================
Texture SkeletalModelLoader::LoadEmbeddedTexture(const aiTexture* etex, const char* typeName)
{
    Texture tex = {};
    tex.type = typeName;

    if (etex->mHeight != 0)
    {
        // Несжатая ARGB8888
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = etex->mWidth; desc.Height = etex->mHeight;
        desc.MipLevels = 1; desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA sd = {};
        sd.pSysMem = etex->pcData;
        sd.SysMemPitch = etex->mWidth * 4;

        ID3D11Texture2D* t2d = nullptr;
        dev_->CreateTexture2D(&desc, &sd, &t2d);
        if (t2d) { dev_->CreateShaderResourceView(t2d, nullptr, &tex.texture); t2d->Release(); }
    }
    else
    {
        CreateWICTextureFromMemory(dev_, ctx_,
            reinterpret_cast<const uint8_t*>(etex->pcData), etex->mWidth,
            nullptr, &tex.texture);
    }

    if (!tex.texture) tex.texture = GetDefaultWhite();
    return tex;
}

// ============================================================
ID3D11ShaderResourceView* SkeletalModelLoader::GetDefaultWhite()
{
    if (defaultWhite_) return defaultWhite_;
    uint32_t white = 0xFFFFFFFF;
    D3D11_TEXTURE2D_DESC d = {};
    d.Width = d.Height = 1; d.MipLevels = d.ArraySize = 1;
    d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d.SampleDesc.Count = 1; d.Usage = D3D11_USAGE_DEFAULT;
    d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA sd = { &white, 4, 4 };
    ID3D11Texture2D* t = nullptr;
    dev_->CreateTexture2D(&d, &sd, &t);
    if (t) { dev_->CreateShaderResourceView(t, nullptr, &defaultWhite_); t->Release(); }
    return defaultWhite_;
}

// ============================================================
ID3D11ShaderResourceView* SkeletalModelLoader::GetDefaultNormal()
{
    if (defaultNormal_) return defaultNormal_;
    uint32_t normal = 0xFFFF8080; // (128,128,255,255) -> нейтральная нормаль
    D3D11_TEXTURE2D_DESC d = {};
    d.Width = d.Height = 1; d.MipLevels = d.ArraySize = 1;
    d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d.SampleDesc.Count = 1; d.Usage = D3D11_USAGE_DEFAULT;
    d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA sd = { &normal, 4, 4 };
    ID3D11Texture2D* t = nullptr;
    dev_->CreateTexture2D(&d, &sd, &t);
    if (t) { dev_->CreateShaderResourceView(t, nullptr, &defaultNormal_); t->Release(); }
    return defaultNormal_;
}

// ============================================================
std::string SkeletalModelLoader::ResolveTexturePath(const aiString& path) const
{
    std::string p = path.C_Str();
    if (p.empty() || p[0] == '*') return p;
    bool isAbs = (p.size() > 1 && p[1] == ':') || (!p.empty() && (p[0] == '\\' || p[0] == '/'));
    if (isAbs || directory_.empty()) return p;
    return directory_ + "\\" + p;
}

// ============================================================
void SkeletalModelLoader::Close()
{
    // НЕ освобождаем defaultWhite_/defaultNormal_ здесь.
    // Их raw-указатели скопированы в поля material каждого меша,
    // и при PSSetShaderResources DX будет их использовать.
    // Просто зануляем — утечки нет, т.к. меши в итоге тоже освободят их через SkeletalMesh::Close().
    defaultWhite_ = nullptr;
    defaultNormal_ = nullptr;
}