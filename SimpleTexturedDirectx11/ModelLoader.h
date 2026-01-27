#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#define NOMINMAX

#include <vector>
#include <d3d11_1.h>
#include <DirectXMath.h>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include "Mesh.h"
#include "TextureLoader.h"

using namespace DirectX;

class ModelLoader
{
public:
	ModelLoader(HWND hwnd, ID3D11Device* dev, ID3D11DeviceContext* devcon);
	~ModelLoader();

	std::vector<Mesh>* Load(std::string filename);

	void Close();
	std::vector<Texture> textures_loaded_;
private:
	ID3D11Device *dev_;
	ID3D11DeviceContext *devcon_;
	std::vector<Mesh> meshes_;
	std::string directory_;
	HWND hwnd_;
	ID3D11ShaderResourceView* defaultWhiteTexture_;
	ID3D11ShaderResourceView* defaultNormalTexture_;

	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	Material loadMaterial(aiMaterial* mat, const aiScene* scene, Texture* outAlbedo);
	Texture loadMaterialTexture(aiMaterial* mat, aiTextureType type, const char* typeName, const aiScene* scene);
	Texture loadTextureFromPath(const std::string& path, const char* typeName, ID3D11ShaderResourceView* fallback);
	Texture loadTextureBySuffix(const std::string& basePath, const std::vector<std::string>& suffixes, const char* typeName, ID3D11ShaderResourceView* fallback);
	ID3D11ShaderResourceView* getDefaultWhiteTexture();
	ID3D11ShaderResourceView* getDefaultNormalTexture();
	std::string resolveTexturePath(const aiString& path) const;
	ID3D11ShaderResourceView* loadEmbeddedTexture(const aiTexture* embeddedTexture);
};

#endif // !MODEL_LOADER_H

