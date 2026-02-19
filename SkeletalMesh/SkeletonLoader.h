#pragma once
#include "Skeleton.h"
#include "SkinnedMesh.h"
#include "AnimationClip.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <d3d11.h>
#include <unordered_set>

// ============================================================================
// SkeletonLoader.h - COMPLETE VERSION
// Loading skeletal data and animations from FBX files using Assimp
// ============================================================================

namespace Animation {

    // ------------------------------------------------------------------------
    // SkeletonLoader - loads skeleton, skinned mesh, and animations from FBX
    // ------------------------------------------------------------------------
    class SkeletonLoader {
    public:
        
        // --------------------------------------------------------------------
        // Load skeleton from Assimp scene
        // --------------------------------------------------------------------
        static Skeleton* LoadSkeleton(const aiScene* scene) {
            if (!scene || !scene->mRootNode) {
                printf("[SkeletonLoader] Error: Invalid scene\n");
                return nullptr;
            }
            
            Skeleton* skeleton = new Skeleton();
            
            // First pass: Find all bones referenced by meshes
            std::unordered_set<std::string> boneNames;
            for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
                aiMesh* mesh = scene->mMeshes[i];
                for (unsigned int j = 0; j < mesh->mNumBones; j++) {
                    boneNames.insert(mesh->mBones[j]->mName.C_Str());
                }
            }
            
            printf("[SkeletonLoader] Found %zu unique bones in meshes\n", boneNames.size());
            
            // Second pass: Build bone hierarchy from scene graph
            std::function<void(aiNode*, int)> processNode;
            processNode = [&](aiNode* node, int parentId) {
                std::string nodeName = node->mName.C_Str();
                
                // Check if this node is a bone
                bool isBone = boneNames.find(nodeName) != boneNames.end();
                
                int currentId = -1;
                if (isBone) {
                    // Create bone
                    Bone bone(nodeName, -1, parentId);
                    
                    // Convert Assimp matrix to DirectX matrix
                    bone.localTransform = AiMatrixToMatrix(node->mTransformation);
                    
                    currentId = skeleton->AddBone(bone);
                    
                    printf("[SkeletonLoader] Added bone: %s (id=%d, parent=%d)\n", 
                           nodeName.c_str(), currentId, parentId);
                }
                
                // Process children
                int childParentId = isBone ? currentId : parentId;
                for (unsigned int i = 0; i < node->mNumChildren; i++) {
                    processNode(node->mChildren[i], childParentId);
                }
            };
            
            // Start from root
            processNode(scene->mRootNode, -1);
            
            // Third pass: Set offset matrices from mesh bone data
            for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
                aiMesh* mesh = scene->mMeshes[i];
                for (unsigned int j = 0; j < mesh->mNumBones; j++) {
                    aiBone* aiBone = mesh->mBones[j];
                    std::string boneName = aiBone->mName.C_Str();
                    
                    Bone* bone = skeleton->GetBone(boneName);
                    if (bone) {
                        bone->offsetMatrix = AiMatrixToMatrix(aiBone->mOffsetMatrix);
                    }
                }
            }
            
            // Update transforms
            skeleton->UpdateTransforms();
            skeleton->UpdateBoneMatrices();
            
            printf("[SkeletonLoader] Loaded skeleton with %zu bones\n", 
                   skeleton->GetBoneCount());
            skeleton->PrintHierarchy();
            
            return skeleton;
        }
        
        // --------------------------------------------------------------------
        // Load skinned mesh from Assimp scene
        // --------------------------------------------------------------------
        static SkinnedMesh* LoadSkinnedMesh(const aiScene* scene, 
                                           Skeleton* skeleton,
                                           ID3D11Device* device,
                                           ID3D11ShaderResourceView* defaultAlbedo = nullptr,
                                           ID3D11ShaderResourceView* defaultNormal = nullptr) {
            if (!scene || !skeleton || scene->mNumMeshes == 0) {
                printf("[SkeletonLoader] Error: Invalid parameters\n");
                return nullptr;
            }
            
            aiMesh* aiMesh = scene->mMeshes[0];  // Load first mesh
            
            printf("[SkeletonLoader] Loading skinned mesh: %s\n", aiMesh->mName.C_Str());
            printf("  Vertices: %u\n", aiMesh->mNumVertices);
            printf("  Faces: %u\n", aiMesh->mNumFaces);
            printf("  Bones: %u\n", aiMesh->mNumBones);
            
            // Load vertices
            std::vector<SkinnedVertex> vertices;
            vertices.resize(aiMesh->mNumVertices);
            
            for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                SkinnedVertex& vertex = vertices[i];
                
                // Position
                vertex.x = aiMesh->mVertices[i].x;
                vertex.y = aiMesh->mVertices[i].y;
                vertex.z = aiMesh->mVertices[i].z;
                
                // Normal
                if (aiMesh->HasNormals()) {
                    vertex.nx = aiMesh->mNormals[i].x;
                    vertex.ny = aiMesh->mNormals[i].y;
                    vertex.nz = aiMesh->mNormals[i].z;
                } else {
                    vertex.nx = 0;
                    vertex.ny = 1;
                    vertex.nz = 0;
                }
                
                // Texture coordinates
                if (aiMesh->HasTextureCoords(0)) {
                    vertex.u = aiMesh->mTextureCoords[0][i].x;
                    vertex.v = aiMesh->mTextureCoords[0][i].y;
                } else {
                    vertex.u = 0;
                    vertex.v = 0;
                }
                
                // Tangent and Bitangent
                if (aiMesh->HasTangentsAndBitangents()) {
                    vertex.tx = aiMesh->mTangents[i].x;
                    vertex.ty = aiMesh->mTangents[i].y;
                    vertex.tz = aiMesh->mTangents[i].z;
                    
                    vertex.bx = aiMesh->mBitangents[i].x;
                    vertex.by = aiMesh->mBitangents[i].y;
                    vertex.bz = aiMesh->mBitangents[i].z;
                } else {
                    // Calculate tangent from normal
                    Vector3 normal(vertex.nx, vertex.ny, vertex.nz);
                    Vector3 tangent = Vector3::UnitX;
                    if (abs(normal.Dot(tangent)) > 0.9f) {
                        tangent = Vector3::UnitY;
                    }
                    tangent = (tangent - normal * normal.Dot(tangent));
                    tangent.Normalize();
                    
                    vertex.tx = tangent.x;
                    vertex.ty = tangent.y;
                    vertex.tz = tangent.z;
                    
                    Vector3 bitangent = normal.Cross(tangent);
                    vertex.bx = bitangent.x;
                    vertex.by = bitangent.y;
                    vertex.bz = bitangent.z;
                }
                
                // Initialize bone weights to zero
                for (int j = 0; j < 4; j++) {
                    vertex.boneIds[j] = 0;
                    vertex.weights[j] = 0.0f;
                }
            }
            
            // Load bone weights
            int totalWeights = 0;
            for (unsigned int i = 0; i < aiMesh->mNumBones; i++) {
                aiBone* bone = aiMesh->mBones[i];
                std::string boneName = bone->mName.C_Str();
                int boneId = skeleton->GetBoneId(boneName);
                
                if (boneId == -1) {
                    printf("[SkeletonLoader] Warning: Bone %s not found in skeleton\n", 
                           boneName.c_str());
                    continue;
                }
                
                // Add weights to vertices
                for (unsigned int j = 0; j < bone->mNumWeights; j++) {
                    unsigned int vertexId = bone->mWeights[j].mVertexId;
                    float weight = bone->mWeights[j].mWeight;
                    
                    if (vertexId >= vertices.size()) continue;
                    
                    AddBoneWeightToVertex(vertices[vertexId], boneId, weight);
                    totalWeights++;
                }
            }
            
            printf("[SkeletonLoader] Loaded %d bone weights\n", totalWeights);
            
            // Normalize weights
            for (auto& vertex : vertices) {
                NormalizeWeights(vertex);
            }
            
            // Load indices
            std::vector<UINT> indices;
            for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
                aiFace& face = aiMesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    indices.push_back(face.mIndices[j]);
                }
            }
            
            // Create material (simple for now)
            SkinnedMaterial material;
            material.albedo = defaultAlbedo;
            material.normal = defaultNormal;
            material.baseColorFactor = XMFLOAT4(1, 1, 1, 1);
            material.materialParams = XMFLOAT4(0, 1, 1, 0);
            
            // Create SkinnedMesh
            printf("[SkeletonLoader] Creating SkinnedMesh...\n");
            return new SkinnedMesh(device, vertices, indices, material, skeleton);
        }
        
        // --------------------------------------------------------------------
        // Load animation from Assimp scene
        // 
        // Parameters:
        //   scene - Assimp scene with animation data
        //   skeleton - Target skeleton
        //   animationIndex - Which animation to load (default: 0)
        // 
        // Returns: AnimationClip or nullptr if failed
        // --------------------------------------------------------------------
        static AnimationClip* LoadAnimation(const aiScene* scene, 
                                           Skeleton* skeleton,
                                           int animationIndex = 0) {
            if (!scene || !skeleton) {
                printf("[SkeletonLoader] Error: Invalid parameters for animation\n");
                return nullptr;
            }
            
            if (animationIndex >= static_cast<int>(scene->mNumAnimations)) {
                printf("[SkeletonLoader] Error: Animation index %d out of range (total: %u)\n",
                       animationIndex, scene->mNumAnimations);
                return nullptr;
            }
            
            aiAnimation* aiAnim = scene->mAnimations[animationIndex];
            
            printf("[SkeletonLoader] Loading animation: %s\n", aiAnim->mName.C_Str());
            printf("  Duration: %.2f ticks\n", aiAnim->mDuration);
            printf("  Ticks per second: %.2f\n", aiAnim->mTicksPerSecond);
            printf("  Channels: %u\n", aiAnim->mNumChannels);
            
            AnimationClip* clip = new AnimationClip();
            clip->name = aiAnim->mName.C_Str();
            clip->duration = static_cast<float>(aiAnim->mDuration / aiAnim->mTicksPerSecond);
            clip->ticksPerSecond = static_cast<float>(aiAnim->mTicksPerSecond);
            
            // Load animation channels (per-bone tracks)
            for (unsigned int i = 0; i < aiAnim->mNumChannels; i++) {
                aiNodeAnim* channel = aiAnim->mChannels[i];
                std::string boneName = channel->mNodeName.C_Str();
                
                int boneId = skeleton->GetBoneId(boneName);
                if (boneId == -1) {
                    printf("[SkeletonLoader] Warning: Animation channel for unknown bone: %s\n",
                           boneName.c_str());
                    continue;
                }
                
                BoneAnimationTrack track;
                track.boneId = boneId;
                
                // Load position keyframes
                for (unsigned int j = 0; j < channel->mNumPositionKeys; j++) {
                    BoneKeyframe key;
                    key.time = static_cast<float>(channel->mPositionKeys[j].mTime / aiAnim->mTicksPerSecond);
                    
                    aiVector3D pos = channel->mPositionKeys[j].mValue;
                    key.position = Vector3(pos.x, pos.y, pos.z);
                    
                    track.positionKeys.push_back(key);
                }
                
                // Load rotation keyframes
                for (unsigned int j = 0; j < channel->mNumRotationKeys; j++) {
                    BoneKeyframe key;
                    key.time = static_cast<float>(channel->mRotationKeys[j].mTime / aiAnim->mTicksPerSecond);
                    
                    aiQuaternion rot = channel->mRotationKeys[j].mValue;
                    key.rotation = Quaternion(rot.x, rot.y, rot.z, rot.w);
                    
                    track.rotationKeys.push_back(key);
                }
                
                // Load scale keyframes
                for (unsigned int j = 0; j < channel->mNumScalingKeys; j++) {
                    BoneKeyframe key;
                    key.time = static_cast<float>(channel->mScalingKeys[j].mTime / aiAnim->mTicksPerSecond);
                    
                    aiVector3D scl = channel->mScalingKeys[j].mValue;
                    key.scale = Vector3(scl.x, scl.y, scl.z);
                    
                    track.scaleKeys.push_back(key);
                }
                
                printf("[SkeletonLoader]   Bone %s: %zu pos, %zu rot, %zu scale keys\n",
                       boneName.c_str(),
                       track.positionKeys.size(),
                       track.rotationKeys.size(),
                       track.scaleKeys.size());
                
                clip->tracks.push_back(track);
            }
            
            printf("[SkeletonLoader] Animation loaded successfully: %.2f seconds, %zu tracks\n",
                   clip->duration, clip->tracks.size());
            
            return clip;
        }
        
    private:
        // Helper: Convert Assimp matrix to DirectX matrix
        static Matrix AiMatrixToMatrix(const aiMatrix4x4& aiMat) {
            return Matrix(
                aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
                aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
                aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
                aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
            );
        }
        
        // Helper: Add bone weight to vertex
        static void AddBoneWeightToVertex(SkinnedVertex& vertex, int boneId, float weight) {
            for (int i = 0; i < 4; i++) {
                if (vertex.weights[i] == 0.0f) {
                    vertex.boneIds[i] = boneId;
                    vertex.weights[i] = weight;
                    return;
                }
            }
            
            // If all slots full, replace smallest weight
            int minIndex = 0;
            float minWeight = vertex.weights[0];
            for (int i = 1; i < 4; i++) {
                if (vertex.weights[i] < minWeight) {
                    minWeight = vertex.weights[i];
                    minIndex = i;
                }
            }
            
            if (weight > minWeight) {
                vertex.boneIds[minIndex] = boneId;
                vertex.weights[minIndex] = weight;
            }
        }
        
        // Helper: Normalize bone weights
        static void NormalizeWeights(SkinnedVertex& vertex) {
            float sum = 0.0f;
            for (int i = 0; i < 4; i++) {
                sum += vertex.weights[i];
            }
            
            if (sum > 0.0001f) {
                for (int i = 0; i < 4; i++) {
                    vertex.weights[i] /= sum;
                }
            }
        }
    };

} // namespace Animation
