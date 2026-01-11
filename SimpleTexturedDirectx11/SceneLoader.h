#pragma once

#include <vector>
#include <d3d11_1.h>
#include <DirectXMath.h>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include "Mesh.h"
#include "TextureLoader.h"
#include "../GameObject.h"
#include "../Game.h"

using namespace DirectX;

class SceneLoader
{
public:
	SceneLoader(Game* game, HWND hwnd, ID3D11Device* dev, ID3D11DeviceContext* devcon);
	~SceneLoader();

	std::vector<GameObject*>* Load(std::string filename);

	void Close();
	std::vector<Texture> textures_loaded_;
private:
	ID3D11Device* dev_;
	ID3D11DeviceContext* devcon_;
	std::vector<GameObject*> gameObjects_;
	std::string directory_;
	HWND hwnd_;
	Game* game_;

	void processNode(aiNode* node, Transform* parent, const aiScene* scene);
	void processLight(aiLight* light, GameObject* gameObject, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene);
	ID3D11ShaderResourceView* loadEmbeddedTexture(const aiTexture* embeddedTexture);
};
