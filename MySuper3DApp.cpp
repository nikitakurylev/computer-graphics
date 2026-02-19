// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "Game.h"
#include "ModelComponent.h"
#include "PointLightComponent.h"
#include "DeferredRenderingSystem.h"
#include "SimpleTexturedDirectx11/ModelLoader.h"
#include "SimpleTexturedDirectx11/SceneLoader.h"
#include "ScriptingEngine.h"
#include "Logger.hpp"
#include "AdvancedEnemyAI.h"

// ============================================================================
// [SKELETAL ANIMATION] Includes
// ============================================================================
#include "SkeletalMesh/SkeletonLoader.h"
#include "SkeletalMesh/SkinnedModelComponent.h"
#include "SkeletalMesh/SkeletalAnimationComponent.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// ============================================================================

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

// ============================================================================
// Helper function: Load FBX file using Assimp
// ============================================================================
const aiScene* LoadFBX(const std::string& filename) {
    static Assimp::Importer importer;

    printf("\n[LoadFBX] Loading file: %s\n", filename.c_str());

    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_LimitBoneWeights |
        aiProcess_FlipUVs
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("[LoadFBX] ERROR: %s\n", importer.GetErrorString());
        return nullptr;
    }

    printf("[LoadFBX] Success!\n");
    printf("  Meshes: %u\n", scene->mNumMeshes);
    printf("  Animations: %u\n", scene->mNumAnimations);
    if (scene->mNumMeshes > 0) {
        printf("  Bones: %u\n", scene->mMeshes[0]->mNumBones);
    }

    return scene;
}

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    auto window = DisplayWin32::Instance();
    auto inputDevice = InputDevice();
    auto render = DeferredRenderingSystem(&window);
    auto logger = ConsoleLogger();
    auto scriptingEngine = ScriptingEngine(&logger);
    auto game = Game(&window, &inputDevice, &render, &scriptingEngine);

    scriptingEngine.Init();
    scriptingEngine.GatherLayouts();

    auto modelLoader = ModelLoader(window.hWnd, render.Device, render.Context);
    auto sceneLoader = SceneLoader(&game, &scriptingEngine, window.hWnd, render.Device, render.Context);

    // ========================================================================
    // Load existing scene (platforms, lights, etc.)
    // ========================================================================
    printf("\n[Main] Loading scene: untitled.glb\n");
    auto sceneObjects = sceneLoader.Load("untitled.glb");
    for (GameObject* gameObject : *sceneObjects) {
        game.GameObjects.push_back(gameObject);
    }
    printf("[Main] Scene loaded. Total objects: %zu\n", game.GameObjects.size());

    // ========================================================================
    // [SKELETAL ANIMATION] Add Y_Bot character to the scene
    // ========================================================================
    printf("\n========================================\n");
    printf("LOADING SKELETAL CHARACTER\n");
    printf("========================================\n");

    // Try to load Y_Bot.fbx
    const aiScene* modelScene = LoadFBX("Y_Bot.fbx");

    if (modelScene) {
        printf("\n[SkeletalAnimation] Loading skeleton...\n");

        // Load skeleton
        auto skeleton = Animation::SkeletonLoader::LoadSkeleton(modelScene);

        if (skeleton) {
            printf("[SkeletalAnimation] Skeleton loaded successfully\n");

            // Load skinned mesh
            printf("[SkeletalAnimation] Loading skinned mesh...\n");
            auto skinnedMesh = Animation::SkeletonLoader::LoadSkinnedMesh(
                modelScene,
                skeleton,
                render.Device,
                nullptr,  // Will use default white texture from rendering system
                nullptr   // Will use default normal texture
            );

            if (skinnedMesh) {
                printf("[SkeletalAnimation] Skinned mesh loaded successfully\n");

                int32_t uid = 10000;
                Vector3 startPos(-15, 5, 0);
                Vector3 startScale(0.6f, 0.6f, 0.6f);
                // Create GameObject for character
                auto scriptingTransform = scriptingEngine.CreateScriptingTransformComponent(uid, startPos, startScale);
                auto characterObj = new GameObject(9999, &game, scriptingTransform);

                // Position the character on a platform
                // Adjust Y position to be on top of a platform
                characterObj->GetTransform()->position = Vector3(0, 0.5f, 0);
                characterObj->GetTransform()->scale = Vector3(0.01f, 0.01f, 0.01f);  // Scale down (FBX units are large)

                // Rotate to face camera (optional - adjust as needed)
                characterObj->GetTransform()->rotation = Quaternion::CreateFromAxisAngle(
                    Vector3::UnitY, DirectX::XM_PI
                );

                // Add skinned model component
                auto skinnedModelComp = new SkinnedModelComponent(skinnedMesh, skeleton);
                characterObj->AddComponent(skinnedModelComp);

                // Add to game
                game.GameObjects.push_back(characterObj);

                printf("[SkeletalAnimation] Character added to scene!\n");
                printf("  Position: (%.2f, %.2f, %.2f)\n",
                    characterObj->GetTransform()->position.x,
                    characterObj->GetTransform()->position.y,
                    characterObj->GetTransform()->position.z);
                printf("  Scale: %.3f\n", characterObj->GetTransform()->scale.x);

                // ============================================================
                // [OPTIONAL] Load and play walking animation
                // ============================================================
                printf("\n[SkeletalAnimation] Loading walking animation...\n");
                const aiScene* animScene = LoadFBX("Walking.fbx");

                if (animScene && animScene->mNumAnimations > 0) {
                    auto walkClip = Animation::SkeletonLoader::LoadAnimation(
                        animScene, skeleton, 0
                    );

                    if (walkClip) {
                        auto animComp = new SkeletalAnimationComponent(skeleton);
                        animComp->SetAnimationClip(walkClip);
                        animComp->SetLoop(true);
                        animComp->Play();
                        characterObj->AddComponent(animComp);

                        printf("[SkeletalAnimation] Walking animation loaded and playing!\n");
                        printf("  Duration: %.2f seconds\n", walkClip->duration);
                        printf("  Looping: YES\n");
                    }
                    else {
                        printf("[SkeletalAnimation] Warning: Failed to load animation clip\n");
                    }
                }
                else {
                    printf("[SkeletalAnimation] Info: Walking.fbx not found or has no animations\n");
                    printf("  Character will be in T-pose (bind pose)\n");
                }
            }
            else {
                printf("[SkeletalAnimation] ERROR: Failed to load skinned mesh\n");
            }
        }
        else {
            printf("[SkeletalAnimation] ERROR: Failed to load skeleton\n");
        }
    }
    else {
        printf("\n[SkeletalAnimation] WARNING: Y_Bot.fbx not found\n");
        printf("  Make sure Y_Bot.fbx is in the same folder as the .exe\n");
        printf("  Continuing without skeletal character...\n");
    }

    printf("========================================\n");
    printf("SKELETAL ANIMATION SETUP COMPLETE\n");
    printf("========================================\n\n");
    // ========================================================================

    // Initialize and run game
    game.Initialize();
    game.Run();
}