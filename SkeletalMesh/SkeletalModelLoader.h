#pragma once

#define NOMINMAX
#include <string>
#include <vector>
#include <unordered_map>
#include <d3d11.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "SkeletalMesh.h"
#include "../SimpleTexturedDirectx11/TextureLoader.h"

// ============================================================
//  Результат загрузки скелетной модели
// ============================================================
struct SkeletalModelData
{
    std::vector<SkeletalMesh> meshes;   // все submesh'и
    Skeleton                  skeleton; // общий скелет
    bool                      valid = false;
};

// ============================================================
//  Загрузчик скелетных моделей из FBX / glTF / etc.
// ============================================================
class SkeletalModelLoader
{
public:
    SkeletalModelLoader(HWND hwnd, ID3D11Device* dev, ID3D11DeviceContext* ctx)
        : hwnd_(hwnd), dev_(dev), ctx_(ctx) {
    }

    ~SkeletalModelLoader() { Close(); }

    // Загружает файл и возвращает данные скелетной модели.
    // Вызывающая сторона владеет результатом.
    SkeletalModelData* Load(const std::string& filename);

    std::vector<Texture> textures_loaded_;

private:
    HWND                  hwnd_;
    ID3D11Device* dev_;
    ID3D11DeviceContext* ctx_;
    std::string           directory_;

    ID3D11ShaderResourceView* defaultWhite_ = nullptr;
    ID3D11ShaderResourceView* defaultNormal_ = nullptr;

    // --- helpers ---
    void ProcessNode(aiNode* node, const aiScene* scene,
        SkeletalModelData* out, const Matrix& parentTransform);

    SkeletalMesh ProcessMesh(aiMesh* mesh, const aiScene* scene,
        SkeletalModelData* out);

    void ExtractBoneHierarchy(aiNode* node, const aiScene* scene,
        Skeleton& skeleton, int parentIndex,
        const Matrix& parentTransform);

    void CollectBoneNames(const aiScene* scene,
        std::unordered_map<std::string, Matrix>& boneOffsets);

    Material    LoadMaterial(aiMaterial* mat, const aiScene* scene);
    Texture     LoadTexture(aiMaterial* mat, aiTextureType type, const char* typeName, const aiScene* scene);
    Texture     LoadEmbeddedTexture(const aiTexture* tex, const char* typeName);

    ID3D11ShaderResourceView* GetDefaultWhite();
    ID3D11ShaderResourceView* GetDefaultNormal();

    std::string ResolveTexturePath(const aiString& path) const;

    void Close();

    static Matrix ToMatrix(const aiMatrix4x4& m)
    {
        // Assimp row-major -> DirectXMath row-major (они одинаковы, просто транспонируем)
        return Matrix(
            m.a1, m.b1, m.c1, m.d1,
            m.a2, m.b2, m.c2, m.d2,
            m.a3, m.b3, m.c3, m.d3,
            m.a4, m.b4, m.c4, m.d4
        );
    }
};