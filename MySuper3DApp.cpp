// MySuper3DApp.cpp : This file contains the 'main' function.

#include "Game.h"
#include "GameObject.h"
#include "Transform.h"
#include "PointLightComponent.h"
#include "DeferredRenderingSystem.h"
#include "ScriptingEngine.h"
#include "Logger.hpp"
#include "BulletComponent.h"
#include "PlaneComponent.h"

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

// ----------------------------------------------------------------
//  Вспомогательная функция — создаёт GameObject с Transform
//  Принимает Vector3 по значению, т.к. CreateScriptingTransformComponent
//  требует Vector3& (не const), поэтому берём non-const локальные копии.
// ----------------------------------------------------------------
static int32_t sUidCounter = 1;

static GameObject* MakeObject(Game& game, ScriptingEngine& se,
    Vector3 pos,
    Vector3 scale,
    const char* name = "Object")
{
    int32_t uid = sUidCounter++;
    // CreateScriptingTransformComponent принимает Vector3& (не const) — передаём локальные переменные
    auto st = se.CreateScriptingTransformComponent(uid, pos, scale);
    se.CreateScriptingGameObject(uid, name);
    auto* obj = new GameObject(uid, &game, st);
    obj->GetTransform()->position = pos;
    obj->GetTransform()->scale = scale;
    obj->GetTransform()->immovable = true;
    return obj;
}

// ================================================================
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

    // ----------------------------------------------------------------
    //  Пол — горизонтальная плоскость 40x40 единиц
    // ----------------------------------------------------------------
    {
        auto* obj = MakeObject(game, scriptingEngine,
            Vector3(0.0f, 0.0f, 0.0f), Vector3::One, "Floor");
        obj->AddComponent(new PlaneComponent(40.0f, 8.0f));
        game.GameObjects.push_back(obj);
    }

    // ----------------------------------------------------------------
    //  Источники света
    // ----------------------------------------------------------------
    {
        // Основной тёплый свет сверху-сбоку
        auto* obj = MakeObject(game, scriptingEngine,
            Vector3(5.0f, 8.0f, 5.0f), Vector3::One, "Light_Main");
        obj->AddComponent(new PointLightComponent(Vector4(1.0f, 0.95f, 0.85f, 1.0f), 30.0f));
        game.GameObjects.push_back(obj);
    }
    {
        // Заполняющий голубоватый свет
        auto* obj = MakeObject(game, scriptingEngine,
            Vector3(-8.0f, 5.0f, -3.0f), Vector3::One, "Light_Fill");
        obj->AddComponent(new PointLightComponent(Vector4(0.3f, 0.5f, 0.9f, 1.0f), 20.0f));
        game.GameObjects.push_back(obj);
    }

    // ----------------------------------------------------------------
    //  Y_Bot — скелетная модель с анимацией ходьбы
    //
    //  ВАЖНО: skelLoader должен жить всё время работы программы —
    //  его defaultWhite/defaultNormal используются в мешах.
    // ----------------------------------------------------------------
    SkeletalModelLoader* skelLoader = new SkeletalModelLoader(window.hWnd, render.Device, render.Context);
    SkeletalModelData* yBotData = skelLoader->Load("Y_Bot.fbx");

    if (yBotData && yBotData->valid)
    {
        // Y_Bot из Mixamo в FBX — сантиметры, поэтому масштаб 0.01
        auto* yBotObject = MakeObject(game, scriptingEngine,
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.01f, 0.01f, 0.01f),
            "YBot");
        yBotObject->GetTransform()->immovable = false;

        auto* skeletalComp = new SkeletalModelComponent(&yBotData->meshes, &yBotData->skeleton);
        yBotObject->AddComponent(skeletalComp);

        DumpSkeleton(yBotData->skeleton, "skeleton_dump.txt");

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
                "Standard_Walk.fbx not found — showing T-pose",
                "Animation Warning", MB_ICONWARNING);
        }

        game.GameObjects.push_back(yBotObject);
    }
    else
    {
        MessageBoxA(window.hWnd,
            "Failed to load Y_Bot.fbx",
            "Warning", MB_ICONWARNING);
    }

    // ----------------------------------------------------------------
    //  Пул шариков (10 штук, переиспользуются по кругу)
    //  Управление: кнопка X — один выстрел за нажатие
    // ----------------------------------------------------------------
    constexpr int kPoolSize = 10;
    for (int i = 0; i < kPoolSize; ++i)
    {
        auto* bulletObj = MakeObject(game, scriptingEngine,
            Vector3(0.0f, -1.0e7f, 0.0f),
            Vector3(0.15f, 0.15f, 0.15f),
            "Bullet");

        auto* bullet = new BulletComponent();
        bulletObj->AddComponent(bullet);
        game.GameObjects.push_back(bulletObj);
        game.RegisterProjectile(bullet);
    }

    // ----------------------------------------------------------------
    game.Initialize();
    game.Run();

    return 0;
}