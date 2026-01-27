#include "SceneLoader.h"
#include "../ModelComponent.h"
#include "../PointLightComponent.h"
#include <assimp/material.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>

#include "../AnimationComponent.h"
#include "../CharacterControllerComponent.h"

#ifndef AI_MATKEY_BASE_COLOR
#define AI_MATKEY_BASE_COLOR "$mat.baseColor", 0, 0
#endif
#ifndef AI_MATKEY_METALLIC_FACTOR
#define AI_MATKEY_METALLIC_FACTOR "$mat.metallicFactor", 0, 0
#endif
#ifndef AI_MATKEY_ROUGHNESS_FACTOR
#define AI_MATKEY_ROUGHNESS_FACTOR "$mat.roughnessFactor", 0, 0
#endif
#ifndef AI_MATKEY_OCCLUSION_STRENGTH
#define AI_MATKEY_OCCLUSION_STRENGTH "$mat.occlusionStrength", 0, 0
#endif

namespace {
bool FileExists(const std::string& path) {
	std::ifstream file(path.c_str(), std::ios::binary);
	return file.good();
}

std::string ToLower(std::string value) {
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return value;
}

bool EndsWith(const std::string& value, const std::string& suffix) {
	if (suffix.size() > value.size()) {
		return false;
	}
	return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

std::string StripSuffixCaseInsensitive(const std::string& stem, const std::vector<std::string>& suffixes) {
	std::string stemLower = ToLower(stem);
	for (const auto& suffix : suffixes) {
		std::string suffixLower = ToLower(suffix);
		if (EndsWith(stemLower, suffixLower)) {
			return stem.substr(0, stem.size() - suffix.size());
		}
	}
	return stem;
}
} // namespace

SceneLoader::SceneLoader(Game* game, ScriptingEngine* scriptingEngine, HWND hwnd, ID3D11Device* dev, ID3D11DeviceContext* devcon) :
	game_(game),
	scripting_engine_(scriptingEngine),
	dev_(dev),
	devcon_(devcon),
	gameObjects_(),
	directory_(),
	textures_loaded_(),
	hwnd_(hwnd),
	defaultWhiteTexture_(nullptr),
	defaultNormalTexture_(nullptr),
	stringToComponent()
{
	created_game_objects_uid_ = 0;
}


SceneLoader::~SceneLoader() {
	// empty
}

std::vector<GameObject*>* SceneLoader::Load(std::string filename) {
	Assimp::Importer importer;
	TCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	SetCurrentDirectory(std::wstring(buffer).substr(0, pos).c_str());
	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_CalcTangentSpace);

	if (pScene == nullptr) {
		auto a = importer.GetErrorString();
		Close();
		return new std::vector<GameObject*>();
	}

	auto lastSlash = filename.find_last_of("/\\");
	if (lastSlash != std::string::npos) {
		this->directory_ = filename.substr(0, lastSlash);
	} else {
		this->directory_.clear();
	}

	processNode(pScene->mRootNode, nullptr, pScene);

	auto result = new std::vector<GameObject*>(gameObjects_);

	Close();

	return result;
}

Mesh SceneLoader::processMesh(aiMesh* mesh, const aiScene* scene) {
	// Data to fill
	std::vector<VERTEX> vertices;
	std::vector<UINT> indices;
	std::vector<Texture> textures;

	// Walk through each of the mesh's vertices
	for (UINT i = 0; i < mesh->mNumVertices; i++) {
		VERTEX vertex;

		vertex.X = mesh->mVertices[i].x;
		vertex.Y = mesh->mVertices[i].y;
		vertex.Z = mesh->mVertices[i].z;

        if (mesh->HasNormals()) {
            vertex.NX = mesh->mNormals[i].x;
            vertex.NY = mesh->mNormals[i].y;
            vertex.NZ = mesh->mNormals[i].z;
        } else {
            vertex.NX = 0.0f;
            vertex.NY = 0.0f;
            vertex.NZ = 0.0f;
        }

		if (mesh->mTextureCoords[0]) {
			vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
		} else {
			vertex.texcoord = XMFLOAT2(0.0f, 0.0f);
		}

		if (mesh->HasTangentsAndBitangents()) {
			vertex.TX = mesh->mTangents[i].x;
			vertex.TY = mesh->mTangents[i].y;
			vertex.TZ = mesh->mTangents[i].z;
			vertex.BX = mesh->mBitangents[i].x;
			vertex.BY = mesh->mBitangents[i].y;
			vertex.BZ = mesh->mBitangents[i].z;
		} else {
			vertex.TX = 0.0f;
			vertex.TY = 0.0f;
			vertex.TZ = 0.0f;
			vertex.BX = 0.0f;
			vertex.BY = 0.0f;
			vertex.BZ = 0.0f;
		}

		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	Material materialData = {};
	Texture albedoTexture = {};
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		materialData = loadMaterial(material, scene, &albedoTexture);
	} else {
		materialData = loadMaterial(nullptr, scene, &albedoTexture);
	}

	if (albedoTexture.texture != nullptr) {
		textures.push_back(albedoTexture);
	}

	return Mesh(dev_, vertices, indices, textures, materialData);
}

Material SceneLoader::loadMaterial(aiMaterial* mat, const aiScene* scene, Texture* outAlbedo) {
	Material material = {};
	ID3D11ShaderResourceView* defaultWhite = getDefaultWhiteTexture();
	ID3D11ShaderResourceView* defaultNormal = getDefaultNormalTexture();

	material.albedo = defaultWhite;
	material.ao = defaultWhite;
	material.metallic = defaultWhite;
	material.roughness = defaultWhite;
	material.normal = defaultNormal;
	material.baseColorFactor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.materialParams = XMFLOAT4(0.0f, 1.0f, 1.0f, 0.0f);

	if (mat != nullptr) {
		aiColor4D baseColor;
		if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_BASE_COLOR, &baseColor) ||
			AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &baseColor)) {
			material.baseColorFactor = XMFLOAT4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
		}

		float opacity = 1.0f;
		if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_OPACITY, &opacity)) {
			material.baseColorFactor.w *= opacity;
		}

		float metallic = 0.0f;
		if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_METALLIC_FACTOR, &metallic)) {
			material.materialParams.x = metallic;
		}

		float roughness = 1.0f;
		if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_ROUGHNESS_FACTOR, &roughness)) {
			material.materialParams.y = roughness;
		}

		float aoStrength = 1.0f;
		if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_OCCLUSION_STRENGTH, &aoStrength)) {
			material.materialParams.z = aoStrength;
		}

		float shininess = 0.0f;
		if (material.materialParams.y == 1.0f &&
			AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &shininess) &&
			shininess > 0.0f) {
			material.materialParams.y = sqrtf(2.0f / (shininess + 2.0f));
		}
	}

	Texture albedoTexture = loadMaterialTexture(mat, aiTextureType_BASE_COLOR, "texture_albedo", scene);
	if (albedoTexture.path.empty()) {
		Texture diffuseTexture = loadMaterialTexture(mat, aiTextureType_DIFFUSE, "texture_diffuse", scene);
		if (!diffuseTexture.path.empty()) {
			albedoTexture = diffuseTexture;
		}
	}
	if (outAlbedo != nullptr) {
		*outAlbedo = albedoTexture;
	}
	material.albedo = albedoTexture.texture;

	const std::vector<std::string> metallicSuffixes = {
		"_metallic", "-metallic", "metallic",
		"_metal", "-metal", "metal",
		"_met", "-met", "met",
		"_metalness", "-metalness", "metalness"
	};
	const std::vector<std::string> roughnessSuffixes = {
		"_roughness", "-roughness", "roughness",
		"_rough", "-rough", "rough",
		"_r", "-r", "r"
	};
	const std::vector<std::string> aoSuffixes = {
		"_ao", "-ao", "ao",
		"_occlusion", "-occlusion", "occlusion",
		"_occ", "-occ", "occ",
		"_ambientocclusion", "-ambientocclusion", "ambientocclusion"
	};
	const std::vector<std::string> normalSuffixes = {
		"_normal", "-normal", "normal",
		"_norm", "-norm", "norm",
		"_n", "-n", "n",
		"_normalmap", "-normalmap", "normalmap"
	};

	Texture aoTexture = loadMaterialTexture(mat, aiTextureType_AMBIENT_OCCLUSION, "texture_ao", scene);
	if (aoTexture.path.empty()) {
		Texture lightmapTexture = loadMaterialTexture(mat, aiTextureType_LIGHTMAP, "texture_ao", scene);
		if (!lightmapTexture.path.empty()) {
			aoTexture = lightmapTexture;
		}
	}
	if (aoTexture.path.empty()) {
		aoTexture = loadTextureBySuffix(albedoTexture.path, aoSuffixes, "texture_ao", defaultWhite);
	}
	material.ao = aoTexture.texture;

	Texture metallicTexture = loadMaterialTexture(mat, aiTextureType_METALNESS, "texture_metallic", scene);
	if (metallicTexture.path.empty()) {
		metallicTexture = loadTextureBySuffix(albedoTexture.path, metallicSuffixes, "texture_metallic", defaultWhite);
	}
	material.metallic = metallicTexture.texture;

	Texture roughnessTexture = loadMaterialTexture(mat, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness", scene);
	if (roughnessTexture.path.empty()) {
		roughnessTexture = loadTextureBySuffix(albedoTexture.path, roughnessSuffixes, "texture_roughness", defaultWhite);
	}
	material.roughness = roughnessTexture.texture;

	Texture normalTexture = loadMaterialTexture(mat, aiTextureType_NORMALS, "texture_normal", scene);
	if (normalTexture.path.empty()) {
		Texture heightTexture = loadMaterialTexture(mat, aiTextureType_HEIGHT, "texture_normal", scene);
		if (!heightTexture.path.empty()) {
			normalTexture = heightTexture;
		}
	}
	if (normalTexture.path.empty()) {
		normalTexture = loadTextureBySuffix(albedoTexture.path, normalSuffixes, "texture_normal", defaultNormal);
	}
	if (normalTexture.path.empty()) {
		normalTexture.texture = defaultNormal;
	}
	material.normal = normalTexture.texture;

	return material;
}

Texture SceneLoader::loadMaterialTexture(aiMaterial* mat, aiTextureType type, const char* typeName, const aiScene* scene) {
	Texture texture = {};
	texture.type = typeName;
	texture.texture = getDefaultWhiteTexture();
	texture.path = "";

	if (mat == nullptr || mat->GetTextureCount(type) == 0) {
		return texture;
	}

	aiString str;
	if (mat->GetTexture(type, 0, &str) != AI_SUCCESS) {
		return texture;
	}

	const std::string pathKey = resolveTexturePath(str);
	for (UINT j = 0; j < textures_loaded_.size(); j++) {
		if (textures_loaded_[j].path == pathKey) {
			return textures_loaded_[j];
		}
	}

	HRESULT hr;
	const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
	if (embeddedTexture != nullptr) {
		texture.texture = loadEmbeddedTexture(embeddedTexture);
	} 
	else {
		std::wstring filenamews = std::wstring(pathKey.begin(), pathKey.end());
		hr = CreateWICTextureFromFile(dev_, devcon_, filenamews.c_str(), nullptr, &texture.texture);
		if (FAILED(hr))
			MessageBox(hwnd_, L"Texture couldn't be loaded", L"Error!", MB_ICONERROR | MB_OK);
	}

	if (texture.texture == nullptr) {
		texture.texture = getDefaultWhiteTexture();
		return texture;
	}

	texture.path = pathKey;
	textures_loaded_.push_back(texture);
	return texture;
}

Texture SceneLoader::loadTextureFromPath(const std::string& path, const char* typeName, ID3D11ShaderResourceView* fallback) {
	Texture texture = {};
	texture.type = typeName;
	texture.texture = fallback;
	texture.path = "";

	if (path.empty() || path[0] == '*') {
		return texture;
	}

	std::string pathKey = path;
	const bool isAbsolute = (pathKey.size() > 1 && pathKey[1] == ':') ||
		(!pathKey.empty() && (pathKey[0] == '\\' || pathKey[0] == '/'));
	if (!isAbsolute && !directory_.empty()) {
		pathKey = directory_ + "\\" + pathKey;
	}

	if (!FileExists(pathKey)) {
		return texture;
	}

	for (const auto& loaded : textures_loaded_) {
		if (loaded.path == pathKey) {
			return loaded;
		}
	}

	std::wstring filenamews = std::wstring(pathKey.begin(), pathKey.end());
	HRESULT hr = CreateWICTextureFromFile(dev_, devcon_, filenamews.c_str(), nullptr, &texture.texture);
	if (FAILED(hr) || texture.texture == nullptr) {
		return texture;
	}

	texture.path = pathKey;
	textures_loaded_.push_back(texture);
	return texture;
}

Texture SceneLoader::loadTextureBySuffix(const std::string& basePath, const std::vector<std::string>& suffixes, const char* typeName, ID3D11ShaderResourceView* fallback) {
	Texture texture = {};
	texture.type = typeName;
	texture.texture = fallback;
	texture.path = "";

	if (basePath.empty() || basePath[0] == '*') {
		return texture;
	}

	const std::vector<std::string> baseSuffixes = {
		"_basecolor", "-basecolor", "basecolor",
		"_albedo", "-albedo", "albedo",
		"_diffuse", "-diffuse", "diffuse",
		"_color", "-color", "color",
		"_col", "-col", "col",
		"_bc", "-bc", "bc"
	};
	const std::vector<std::string> exts = { ".png", ".jpg", ".jpeg", ".tga", ".bmp" };

	const size_t slash = basePath.find_last_of("/\\");
	const std::string dir = (slash == std::string::npos) ? std::string() : basePath.substr(0, slash);
	const std::string filename = (slash == std::string::npos) ? basePath : basePath.substr(slash + 1);
	const size_t dot = filename.find_last_of('.');
	const std::string ext = (dot == std::string::npos) ? std::string() : filename.substr(dot);
	const std::string stem = (dot == std::string::npos) ? filename : filename.substr(0, dot);

	const std::string baseStem = StripSuffixCaseInsensitive(stem, baseSuffixes);
	const std::string baseDir = !dir.empty() ? dir : directory_;

	std::vector<std::string> candidateExts;
	if (!ext.empty()) {
		candidateExts.push_back(ext);
	}
	for (const auto& e : exts) {
		if (std::find(candidateExts.begin(), candidateExts.end(), e) == candidateExts.end()) {
			candidateExts.push_back(e);
		}
	}

	for (const auto& suffix : suffixes) {
		for (const auto& candidateExt : candidateExts) {
			std::string candidate = baseStem + suffix + candidateExt;
			if (!baseDir.empty()) {
				candidate = baseDir + "\\" + candidate;
			}
			if (FileExists(candidate)) {
				return loadTextureFromPath(candidate, typeName, fallback);
			}
		}
	}

	return texture;
}

ID3D11ShaderResourceView* SceneLoader::getDefaultWhiteTexture() {
	if (defaultWhiteTexture_ != nullptr) {
		return defaultWhiteTexture_;
	}

	const UINT whitePixel = 0xFFFFFFFF;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = &whitePixel;
	subresourceData.SysMemPitch = sizeof(UINT);

	ID3D11Texture2D* texture2D = nullptr;
	HRESULT hr = dev_->CreateTexture2D(&desc, &subresourceData, &texture2D);
	if (FAILED(hr)) {
		MessageBox(hwnd_, L"Default texture creation failed", L"Error!", MB_ICONERROR | MB_OK);
		return nullptr;
	}

	hr = dev_->CreateShaderResourceView(texture2D, nullptr, &defaultWhiteTexture_);
	texture2D->Release();
	if (FAILED(hr)) {
		MessageBox(hwnd_, L"Default texture SRV creation failed", L"Error!", MB_ICONERROR | MB_OK);
		return nullptr;
	}

	return defaultWhiteTexture_;
}

ID3D11ShaderResourceView* SceneLoader::getDefaultNormalTexture() {
	if (defaultNormalTexture_ != nullptr) {
		return defaultNormalTexture_;
	}

	const unsigned char normalPixel[4] = { 128, 128, 255, 255 };

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = normalPixel;
	subresourceData.SysMemPitch = 4;

	ID3D11Texture2D* texture2D = nullptr;
	HRESULT hr = dev_->CreateTexture2D(&desc, &subresourceData, &texture2D);
	if (FAILED(hr)) {
		MessageBox(hwnd_, L"Default normal texture creation failed", L"Error!", MB_ICONERROR | MB_OK);
		return nullptr;
	}

	hr = dev_->CreateShaderResourceView(texture2D, nullptr, &defaultNormalTexture_);
	texture2D->Release();
	if (FAILED(hr)) {
		MessageBox(hwnd_, L"Default normal SRV creation failed", L"Error!", MB_ICONERROR | MB_OK);
		return nullptr;
	}

	return defaultNormalTexture_;
}

std::string SceneLoader::resolveTexturePath(const aiString& path) const {
	std::string pathStr = path.C_Str();
	if (pathStr.empty() || pathStr[0] == '*') {
		return pathStr;
	}

	const bool isAbsolute = (pathStr.size() > 1 && pathStr[1] == ':') ||
		(!pathStr.empty() && (pathStr[0] == '\\' || pathStr[0] == '/'));
	if (isAbsolute || directory_.empty()) {
		return pathStr;
	}

	return directory_ + "\\" + pathStr;
}

void SceneLoader::Close() {
	//for (auto& t : textures_loaded_)
	//	t.Release();

	//for (size_t i = 0; i < meshes_.size(); i++) {
	//	meshes_[i].Close();
	//}
	gameObjects_ = std::vector<GameObject*>();
}

void SceneLoader::processNode(aiNode* node, Transform* parent, const aiScene* scene) 
{
	aiVector3D aiPosition;
	aiQuaternion aiRotation;
	aiVector3D aiScale;

	node->mTransformation.Decompose(aiScale, aiRotation, aiPosition);

	auto position = Vector3(aiPosition.x, aiPosition.y, aiPosition.z);
	auto scale = Vector3(aiScale.x, aiScale.y, aiScale.z);

	int32_t localUid = created_game_objects_uid_;
	std::string gameObjectName = std::string("GameObject") + std::to_string(localUid);
	
	auto scriptingTransformComponent = scripting_engine_->CreateScriptingTransformComponent(localUid, position, scale);
	scripting_engine_->CreateScriptingGameObject(localUid, gameObjectName);
	
	auto gameObject = new GameObject(localUid, game_, scriptingTransformComponent);
	created_game_objects_uid_++;

	auto transform = gameObject->GetTransform();
	transform->position = position;
	transform->scale = scale;
	transform->rotation = Quaternion(aiRotation.x, aiRotation.y, aiRotation.z, aiRotation.w);
	transform->parent = parent;

	if (node->mNumMeshes > 0) {

		auto meshes = new std::vector<Mesh>();

		for (UINT i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes->push_back(this->processMesh(mesh, scene));
		}
		auto modelComponent = new ModelComponent(meshes);
		gameObject->AddComponent(modelComponent);
	}

	for (UINT i = 0; i < scene->mNumLights; i++) {
		if (scene->mLights[i]->mName == node->mName) {
			this->processLight(scene->mLights[i], gameObject, scene);
		}
	}

	for (UINT i = 0; i < scene->mNumAnimations; i++) {
		for (UINT j = 0; j < scene->mAnimations[i]->mNumChannels; j++) {
			if (scene->mAnimations[i]->mChannels[j]->mNodeName == node->mName) {
				this->processAnimation(scene->mAnimations[i]->mChannels[j], gameObject);
			}
		}
	}

	for (UINT i = 0; i < node->mNumChildren; i++) {
		this->processNode(node->mChildren[i], transform, scene);
	}
	
	if(node->mMetaData)
		for (unsigned i = 0; i < node->mMetaData->mNumProperties; i++) {
			const aiString& key = node->mMetaData->mKeys[i];
			const aiMetadataEntry& entry = node->mMetaData->mValues[i];
			if (strcmp(key.C_Str(), "component"))
				continue;

			aiString value;
			node->mMetaData->Get(key, value);

			std::string componentName = std::string(value.C_Str());
			if (componentName == "CharacterControllerComponent") {

				gameObject->AddComponent(new CharacterControllerComponent());
				continue;
			}
			if (componentName == "PhysicsComponent") {

				gameObject->AddComponent(new PhysicsComponent());
				continue;
			}
			if (componentName == "DynamicPhysicsComponent") {

				gameObject->AddComponent(new DynamicPhysicsComponent());
				continue;
			}
				
			auto scriptingComponent = scripting_engine_->CreateComponentForObjectByName(localUid, componentName);
			
			gameObject->AddScriptingComponent(scriptingComponent);
		}

	gameObjects_.push_back(gameObject);
}

void SceneLoader::processLight(aiLight* light, GameObject* gameObject, const aiScene* scene) {

	switch (light->mType)
	{
	case aiLightSource_POINT:
		gameObject->AddComponent(new PointLightComponent(Vector4(light->mColorDiffuse.r / 5300, light->mColorDiffuse.g / 5300, light->mColorDiffuse.b / 5300, 1), 10));
		break;
	default:
		break;
	}
}

void SceneLoader::processAnimation(aiNodeAnim* animation, GameObject* gameObject) {
	auto positions = std::vector<TimeKey<Vector3>>(animation->mNumPositionKeys);

	for (UINT i = 0; i < animation->mNumPositionKeys; i++) {
		positions[i] = TimeKey<Vector3>(animation->mPositionKeys[i].mTime / 1000.0,
			Vector3(animation->mPositionKeys[i].mValue.x, animation->mPositionKeys[i].mValue.y, animation->mPositionKeys[i].mValue.z));
	}
	
	auto scales = std::vector<TimeKey<Vector3>>(animation->mNumScalingKeys);

	for (UINT i = 0; i < animation->mNumScalingKeys; i++) {
		scales[i] = TimeKey<Vector3>(animation->mScalingKeys[i].mTime / 1000.0,
			Vector3(animation->mScalingKeys[i].mValue.x, animation->mScalingKeys[i].mValue.y, animation->mScalingKeys[i].mValue.z));
	}
	
	auto rotations = std::vector<TimeKey<Quaternion>>(animation->mNumRotationKeys);

	for (UINT i = 0; i < animation->mNumRotationKeys; i++) {
		rotations[i] = TimeKey<Quaternion>(animation->mRotationKeys[i].mTime / 1000.0,
			Quaternion(animation->mRotationKeys[i].mValue.x, animation->mRotationKeys[i].mValue.y, animation->mRotationKeys[i].mValue.z, animation->mRotationKeys[i].mValue.w));
	}

	gameObject->AddComponent(new AnimationComponent(positions, scales, rotations));
}

ID3D11ShaderResourceView* SceneLoader::loadEmbeddedTexture(const aiTexture* embeddedTexture) {
	HRESULT hr;
	ID3D11ShaderResourceView* texture = nullptr;

	if (embeddedTexture->mHeight != 0) {
		// Load an uncompressed ARGB8888 embedded texture
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = embeddedTexture->mWidth;
		desc.Height = embeddedTexture->mHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA subresourceData;
		subresourceData.pSysMem = embeddedTexture->pcData;
		subresourceData.SysMemPitch = embeddedTexture->mWidth * 4;
		subresourceData.SysMemSlicePitch = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;

		ID3D11Texture2D* texture2D = nullptr;
		hr = dev_->CreateTexture2D(&desc, &subresourceData, &texture2D);
		if (FAILED(hr))
			MessageBox(hwnd_, L"CreateTexture2D failed!", L"Error!", MB_ICONERROR | MB_OK);

		hr = dev_->CreateShaderResourceView(texture2D, nullptr, &texture);
		if (FAILED(hr))
			MessageBox(hwnd_, L"CreateShaderResourceView failed!", L"Error!", MB_ICONERROR | MB_OK);

		return texture;
	}

	// mHeight is 0, so try to load a compressed texture of mWidth bytes
	const size_t size = embeddedTexture->mWidth;

	hr = CreateWICTextureFromMemory(dev_, devcon_, reinterpret_cast<const unsigned char*>(embeddedTexture->pcData), size, nullptr, &texture);
	if (FAILED(hr))
		MessageBox(hwnd_, L"Texture couldn't be created from memory!", L"Error!", MB_ICONERROR | MB_OK);

	return texture;
}
