// ============================================================================
// TestSkeletalAnimation.cpp
// Test scene for skeletal animation system
// 
// This file should be integrated into your main application
// Replace the platformer game initialization with this code
// ============================================================================

#include "../Game.h"
#include "../GameObject.h"
#include "SkeletonLoader.h"
#include "SkinnedModelComponent.h"
#include "SkeletalAnimationComponent.h"
#include "../ModelComponent.h"
#include "../CubeComponent.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// ============================================================================
// Helper: Load FBX file using Assimp
// ============================================================================
const aiScene* LoadFBX(const std::string& filename) {
    static Assimp::Importer importer;
    
    printf("[TestScene] Loading FBX: %s\n", filename.c_str());
    
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_LimitBoneWeights |
        aiProcess_FlipUVs
    );
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("[TestScene] Error loading FBX: %s\n", importer.GetErrorString());
        return nullptr;
    }
    
    printf("[TestScene] FBX loaded successfully\n");
    printf("  Meshes: %u\n", scene->mNumMeshes);
    printf("  Bones: %u (in first mesh)\n", 
           scene->mNumMeshes > 0 ? scene->mMeshes[0]->mNumBones : 0);
    printf("  Animations: %u\n", scene->mNumAnimations);
    
    return scene;
}

// ============================================================================
// Create test scene with skeletal model
// ============================================================================
void CreateSkeletalTestScene(Game* game, ID3D11Device* device, 
                             ID3D11DeviceContext* context,
                             ID3D11ShaderResourceView* defaultWhite,
                             ID3D11ShaderResourceView* defaultNormal) {
    
    printf("\n========================================\n");
    printf("Creating Skeletal Animation Test Scene\n");
    printf("========================================\n\n");
    
    // ------------------------------------------------------------------------
    // 1. Create ground platform
    // ------------------------------------------------------------------------
    printf("[TestScene] Creating ground platform...\n");
    
    auto groundObj = new GameObject(1000, game, nullptr);
    groundObj->GetTransform()->position = Vector3(0, -2, 0);
    groundObj->GetTransform()->scale = Vector3(20, 0.5f, 20);  // Wide platform
    
    auto groundRenderer = new CubeComponent();
    groundObj->AddComponent(groundRenderer);
    
    game->GameObjects.push_back(groundObj);
    
    // ------------------------------------------------------------------------
    // 2. Load skeletal model (Y_Bot.fbx)
    // ------------------------------------------------------------------------
    printf("[TestScene] Loading skeletal model...\n");
    
    const aiScene* modelScene = LoadFBX("Y_Bot.fbx");
    if (!modelScene) {
        printf("[TestScene] ERROR: Failed to load Y_Bot.fbx\n");
        return;
    }
    
    // Load skeleton
    auto skeleton = Animation::SkeletonLoader::LoadSkeleton(modelScene);
    if (!skeleton) {
        printf("[TestScene] ERROR: Failed to load skeleton\n");
        return;
    }
    
    // Load skinned mesh
    auto skinnedMesh = Animation::SkeletonLoader::LoadSkinnedMesh(
        modelScene, skeleton, device, defaultWhite, defaultNormal
    );
    if (!skinnedMesh) {
        printf("[TestScene] ERROR: Failed to load skinned mesh\n");
        delete skeleton;
        return;
    }
    
    printf("[TestScene] Skeletal model loaded successfully!\n");
    
    // ------------------------------------------------------------------------
    // 3. Create GameObject for character
    // ------------------------------------------------------------------------
    auto characterObj = new GameObject(1001, game, nullptr);
    characterObj->GetTransform()->position = Vector3(0, 0, 0);
    characterObj->GetTransform()->scale = Vector3(0.01f, 0.01f, 0.01f);  // Scale down (FBX units)
    characterObj->GetTransform()->rotation = Quaternion::CreateFromAxisAngle(
        Vector3::UnitY, DirectX::XM_PI);  // Face camera
    
    // Add rendering component
    auto skinnedModelComp = new SkinnedModelComponent(skinnedMesh, skeleton);
    characterObj->AddComponent(skinnedModelComp);
    
    game->GameObjects.push_back(characterObj);
    
    printf("[TestScene] Character GameObject created\n");
    
    // ------------------------------------------------------------------------
    // 4. Load and play animation (Walking.fbx) - OPTIONAL FOR NOW
    // ------------------------------------------------------------------------
    // We'll test static pose first, then add animation
    
    printf("\n[TestScene] Scene created successfully!\n");
    printf("You should see:\n");
    printf("  - Wide platform at Y=-2\n");
    printf("  - Y_Bot character at origin\n");
    printf("  - Character in T-pose (bind pose)\n\n");
    
    printf("========================================\n");
    printf("Test Scene Ready!\n");
    printf("========================================\n\n");
}

// ============================================================================
// PHASE 2: Add animation (call this after verifying static model works)
// ============================================================================
void AddWalkingAnimation(Game* game, GameObject* characterObj, 
                        Animation::Skeleton* skeleton,
                        ID3D11Device* device) {
    
    printf("\n========================================\n");
    printf("Adding Walking Animation\n");
    printf("========================================\n\n");
    
    // Load animation file
    const aiScene* animScene = LoadFBX("Walking.fbx");
    if (!animScene) {
        printf("[TestScene] ERROR: Failed to load Walking.fbx\n");
        return;
    }
    
    // Load animation clip
    auto walkClip = Animation::SkeletonLoader::LoadAnimation(animScene, skeleton, 0);
    if (!walkClip) {
        printf("[TestScene] ERROR: Failed to load animation clip\n");
        return;
    }
    
    // Create animation component
    auto animComp = new SkeletalAnimationComponent(skeleton);
    animComp->SetAnimationClip(walkClip);
    animComp->SetLoop(true);
    animComp->Play();
    
    characterObj->AddComponent(animComp);
    
    printf("[TestScene] Walking animation added and playing!\n");
    printf("========================================\n\n");
}

// ============================================================================
// INTEGRATION GUIDE
// ============================================================================
/*

TO INTEGRATE INTO YOUR GAME:

1. In MySuper3DApp.cpp or equivalent, REPLACE the old game initialization:

    // OLD CODE (remove platformer game):
    // CreatePlatformerGame(...)
    
    // NEW CODE:
    CreateSkeletalTestScene(&game, render.Device, render.Context, 
                           defaultWhite, defaultNormal);

2. Make sure files are in the right place:
    - Y_Bot.fbx in executable directory
    - Walking.fbx in executable directory

3. IMPORTANT: Compile SkinnedVertexShader.hlsl
    - Add to RenderingSystem initialization
    - Create input layout for SkinnedVertex
    - See next file for details

4. Test in stages:
    STAGE 1: Static model (T-pose)
    - Run game
    - Should see Y_Bot in bind pose on platform
    - Verify skeleton is loaded (check console output)
    
    STAGE 2: Add animation
    - Uncomment AddWalkingAnimation() call
    - Should see Y_Bot walking in place
    
    STAGE 3: Move character
    - Add keyboard input to move characterObj transform
    - Character should walk while moving

5. Camera setup:
    - Position camera to see character
    - Recommended: cam_pos = Vector3(0, 2, -10)
    - Look at: Vector3(0, 0, 0)

*/
