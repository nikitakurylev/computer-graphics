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

// --- Skeletal model + animation support ---
#include "SkeletalMesh/SkeletalModelLoader.h"
#include "SkeletalMesh/SkeletalModelComponent.h"
#include "SkeletalMesh/SkeletalAnimationLoader.h"
#include "SkeletalMesh/SkeletalAnimatorComponent.h"
#include "SkeletalMesh/SkeletalDebug.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


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

    // Загружаем сцену
    auto sceneObjects = sceneLoader.Load("untitled.glb");
    for (GameObject* gameObject : *sceneObjects)
        game.GameObjects.push_back(gameObject);

    // ====================================================================
    //  Y_Bot — скелетная модель в T-pose
    // ====================================================================
    // ВАЖНО: skelLoader должен жить всё время работы программы,
    // т.к. defaultWhite/defaultNormal текстуры из него используются мешами.
    // Выносим его на уровень main, используем new (или static).
    SkeletalModelLoader* skelLoader = new SkeletalModelLoader(window.hWnd, render.Device, render.Context);
    SkeletalModelData* yBotData = skelLoader->Load("Y_Bot.fbx");

    if (yBotData && yBotData->valid)
    {
        static int32_t skeletalUidCounter = 20000;
        int32_t uid = skeletalUidCounter++;

        Vector3 startPos(0.0f, 2.0f, 0.0f);
        // Y_Bot из Mixamo в FBX хранится в сантиметрах → масштаб 0.01
        Vector3 startScale(0.01f, 0.01f, 0.01f);

        auto scriptingTransform = scriptingEngine.CreateScriptingTransformComponent(uid, startPos, startScale);
        scriptingEngine.CreateScriptingGameObject(uid, "YBot");

        auto yBotObject = new GameObject(uid, &game, scriptingTransform);
        yBotObject->GetTransform()->position = startPos;
        yBotObject->GetTransform()->scale = startScale;

        auto* skeletalComp = new SkeletalModelComponent(&yBotData->meshes, &yBotData->skeleton);
        yBotObject->AddComponent(skeletalComp);

        // --- Дамп скелета для диагностики ---
        DumpSkeleton(yBotData->skeleton, "skeleton_dump.txt");
        
        // --- Загружаем анимацию Walking ---
        auto walkClips = SkeletalAnimationLoader::Load("Standard_Walk.fbx");
        if (!walkClips.empty())
        {
            static std::vector<SkeletalAnimationClip> storedClips = std::move(walkClips);
            DumpAnimClip(storedClips[0], "anim_dump.txt");

            auto* animator = new SkeletalAnimatorComponent(&yBotData->skeleton);
            animator->SetClip(&storedClips[0]);
            animator->loop = true;
            animator->speed = 1.0f;
            yBotObject->AddComponent(animator);
        }
        else
        {
            MessageBoxA(window.hWnd,
                "Walking.fbx not found or has no animations — showing T-pose",
                "Animation Warning", MB_ICONWARNING);
        }
        

        game.GameObjects.push_back(yBotObject);
    }
    else
    {
        MessageBoxA(window.hWnd, "Failed to load Y_Bot.fbx — check that the file is in the exe directory", "Warning", MB_ICONWARNING);
    }

    // ====================================================================
    //  Враги (прежний код)
    // ====================================================================
    auto enemyModel = modelLoader.Load("soccer_ball.obj");

    static int32_t enemyUidCounter = 10000;

    // --- ВРАГ 1: CURIOUS (Красный) ---
    {
        int32_t uid = enemyUidCounter++;
        Vector3 startPos(-15, 5, 0);
        Vector3 startScale(0.6f, 0.6f, 0.6f);

        auto scriptingTransform = scriptingEngine.CreateScriptingTransformComponent(uid, startPos, startScale);
        scriptingEngine.CreateScriptingGameObject(uid, "CuriousEnemy");

        auto enemy = new GameObject(uid, &game, scriptingTransform);
        enemy->GetTransform()->position = startPos;
        enemy->GetTransform()->scale = startScale;

        enemy->AddComponent(new ModelComponent(enemyModel));
        enemy->AddComponent(new PointLightComponent(Vector4(1, 0, 0, 1), 20));

        auto ai = new AI::AdvancedEnemyAI(AI::EnemyType::Curious);
        ai->SetMoveSpeed(4.0f);
        ai->SetDetectionRange(25.0f);
        ai->SetComfortDistance(10.0f);
        enemy->AddComponent(ai);

        game.GameObjects.push_back(enemy);
    }

    // --- ВРАГ 2: COLLECTOR (Зелёный) ---
    {
        int32_t uid = enemyUidCounter++;
        Vector3 startPos(15, 5, 0);
        Vector3 startScale(0.6f, 0.6f, 0.6f);

        auto scriptingTransform = scriptingEngine.CreateScriptingTransformComponent(uid, startPos, startScale);
        scriptingEngine.CreateScriptingGameObject(uid, "CollectorEnemy");

        auto enemy = new GameObject(uid, &game, scriptingTransform);
        enemy->GetTransform()->position = startPos;
        enemy->GetTransform()->scale = startScale;

        enemy->AddComponent(new ModelComponent(enemyModel));
        enemy->AddComponent(new PointLightComponent(Vector4(0, 1, 0, 1), 20));

        auto ai = new AI::AdvancedEnemyAI(AI::EnemyType::Collector);
        ai->SetMoveSpeed(5.0f);
        ai->SetDetectionRange(30.0f);
        enemy->AddComponent(ai);

        game.GameObjects.push_back(enemy);
    }

    game.Initialize();
    game.Run();
}