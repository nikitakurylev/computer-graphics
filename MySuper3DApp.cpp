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
    for (GameObject* gameObject : *sceneObjects) {
        game.GameObjects.push_back(gameObject);
    }

    auto enemyModel = modelLoader.Load("soccer_ball.obj");

    // Счётчик UID для врагов
    static int32_t enemyUidCounter = 10000;

    // ------------------------------------------------------------------------
    // ВРАГ 1: CURIOUS (Красный) - Любопытный
    // ------------------------------------------------------------------------
    {
        int32_t uid = enemyUidCounter++;
        Vector3 startPos(-15, 5, 0);
        Vector3 startScale(0.6f, 0.6f, 0.6f);

        auto scriptingTransform = scriptingEngine.CreateScriptingTransformComponent(uid, startPos, startScale);
        scriptingEngine.CreateScriptingGameObject(uid, "CuriousEnemy");

        auto enemy = new GameObject(uid, &game, scriptingTransform);
        enemy->GetTransform()->position = startPos;
        enemy->GetTransform()->scale = startScale;

        // Модель и свет
        enemy->AddComponent(new ModelComponent(enemyModel));
        enemy->AddComponent(new PointLightComponent(Vector4(1, 0, 0, 1), 20)); // Красный

        // AI
        auto ai = new AI::AdvancedEnemyAI(AI::EnemyType::Curious);
        ai->SetMoveSpeed(4.0f);
        ai->SetDetectionRange(25.0f);
        ai->SetComfortDistance(10.0f);
        enemy->AddComponent(ai);

        game.GameObjects.push_back(enemy);
    }

    // ------------------------------------------------------------------------
    // ВРАГ 2: COLLECTOR (Зелёный) - Собиратель
    // ------------------------------------------------------------------------
    {
        int32_t uid = enemyUidCounter++;
        Vector3 startPos(15, 5, 0);
        Vector3 startScale(0.6f, 0.6f, 0.6f);

        auto scriptingTransform = scriptingEngine.CreateScriptingTransformComponent(uid, startPos, startScale);
        scriptingEngine.CreateScriptingGameObject(uid, "CollectorEnemy");

        auto enemy = new GameObject(uid, &game, scriptingTransform);
        enemy->GetTransform()->position = startPos;
        enemy->GetTransform()->scale = startScale;

        // Модель и свет
        enemy->AddComponent(new ModelComponent(enemyModel));
        enemy->AddComponent(new PointLightComponent(Vector4(0, 1, 0, 1), 20)); // Зелёный

        // AI
        auto ai = new AI::AdvancedEnemyAI(AI::EnemyType::Collector);
        ai->SetMoveSpeed(5.0f);
        ai->SetDetectionRange(30.0f);
        enemy->AddComponent(ai);

        game.GameObjects.push_back(enemy);
    }

    game.Initialize();
    game.Run();
}