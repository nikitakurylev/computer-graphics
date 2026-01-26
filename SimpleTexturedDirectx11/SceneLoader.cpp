#include "SceneLoader.h"
#include "../ModelComponent.h"
#include "../PointLightComponent.h"
#include "../AnimationComponent.h"
#include "../KatamariComponent.h"

SceneLoader::SceneLoader(Game* game, ScriptingEngine* scriptingEngine, HWND hwnd, ID3D11Device* dev, ID3D11DeviceContext* devcon) :
	game_(game),
	scripting_engine_(scriptingEngine),
	dev_(dev),
	devcon_(devcon),
	gameObjects_(),
	directory_(),
	textures_loaded_(),
	hwnd_(hwnd),
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
		aiProcess_ConvertToLeftHanded);

	if (pScene == nullptr) {
		auto a = importer.GetErrorString();
		Close();
		return new std::vector<GameObject*>();
	}

	this->directory_ = filename.substr(0, filename.find_last_of("/\\"));

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

		vertex.NX = mesh->mNormals[i].x;
		vertex.NY = mesh->mNormals[i].y;
		vertex.NZ = mesh->mNormals[i].z;
		
		if (mesh->mTextureCoords[0]) {
			vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	}

	return Mesh(dev_, vertices, indices, textures);
}

std::vector<Texture> SceneLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene) {
	std::vector<Texture> textures;
	for (UINT i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for (UINT j = 0; j < textures_loaded_.size(); j++) {
			if (std::strcmp(textures_loaded_[j].path.c_str(), str.C_Str()) == 0) {
				textures.push_back(textures_loaded_[j]);
				skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if (!skip) {   // If texture hasn't been loaded already, load it
			HRESULT hr;
			Texture texture;

			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
			if (embeddedTexture != nullptr) {
				texture.texture = loadEmbeddedTexture(embeddedTexture);
			}
			else {
				std::string filename = std::string(str.C_Str());
				filename = filename;
				std::wstring filenamews = std::wstring(filename.begin(), filename.end());
				hr = CreateWICTextureFromFile(dev_, devcon_, filenamews.c_str(), nullptr, &texture.texture);
				if (FAILED(hr))
					MessageBox(hwnd_, L"Texture couldn't be loaded", L"Error!", MB_ICONERROR | MB_OK);
			}
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			this->textures_loaded_.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}
	return textures;
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
