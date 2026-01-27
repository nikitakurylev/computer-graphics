// ============================================================================
// TestStage1.cpp
// Тестирование базовых структур AI системы (Этап 1)
// ============================================================================

#include "TestStage1.h"
#include <iostream>
#include "BTNodeState.h"
#include "Blackboard.h"
#include "BTNode.h"
#include "SimpleMath.h"

using namespace AI;
using namespace DirectX::SimpleMath;

// ----------------------------------------------------------------------------
// Простой тестовый узел для проверки базового функционала
// ----------------------------------------------------------------------------
class TestNode : public BTNode {
public:
    TestNode(const std::string& name, BTNodeState returnState)
        : BTNode(name), stateToReturn(returnState) {
    }

protected:
    void OnEnter(GameObject* gameObject, Blackboard* blackboard) override {
        std::cout << "[" << GetName() << "] OnEnter called" << std::endl;
    }

    BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) override {
        std::cout << "[" << GetName() << "] Tick called, returning: "
            << BTNodeStateToString(stateToReturn) << std::endl;
        return stateToReturn;
    }

    void OnExit(GameObject* gameObject, Blackboard* blackboard) override {
        std::cout << "[" << GetName() << "] OnExit called" << std::endl;
    }

private:
    BTNodeState stateToReturn;
};

// ----------------------------------------------------------------------------
// Тестирование Blackboard
// ----------------------------------------------------------------------------
static void TestBlackboard() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 1: Blackboard System" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Blackboard blackboard;

    // Тест 1: Запись и чтение разных типов данных
    std::cout << "Test 1.1: Writing different types..." << std::endl;
    blackboard.SetValue("PlayerPosition", Vector3(10.0f, 0.0f, 5.0f));
    blackboard.SetValue("EnemyHealth", 100.0f);
    blackboard.SetValue("IsAlerted", true);
    blackboard.SetValue("PatrolIndex", 3);
    blackboard.SetValue("TargetName", std::string("Player"));

    // Тест 2: Чтение данных
    std::cout << "\nTest 1.2: Reading data..." << std::endl;
    Vector3 playerPos = blackboard.GetValue<Vector3>("PlayerPosition");
    float health = blackboard.GetValue<float>("EnemyHealth");
    bool isAlerted = blackboard.GetValue<bool>("IsAlerted");
    int patrolIdx = blackboard.GetValue<int>("PatrolIndex");
    std::string targetName = blackboard.GetValue<std::string>("TargetName");

    std::cout << "PlayerPosition: (" << playerPos.x << ", " << playerPos.y
        << ", " << playerPos.z << ")" << std::endl;
    std::cout << "EnemyHealth: " << health << std::endl;
    std::cout << "IsAlerted: " << (isAlerted ? "true" : "false") << std::endl;
    std::cout << "PatrolIndex: " << patrolIdx << std::endl;
    std::cout << "TargetName: " << targetName << std::endl;

    // Тест 3: Проверка существования ключей
    std::cout << "\nTest 1.3: Checking key existence..." << std::endl;
    std::cout << "Has 'PlayerPosition': " << (blackboard.HasValue("PlayerPosition") ? "YES" : "NO") << std::endl;
    std::cout << "Has 'NonExistentKey': " << (blackboard.HasValue("NonExistentKey") ? "YES" : "NO") << std::endl;

    // Тест 4: Значения по умолчанию
    std::cout << "\nTest 1.4: Default values..." << std::endl;
    float nonExistent = blackboard.GetValue<float>("NonExistentKey", 999.0f);
    std::cout << "NonExistentKey with default: " << nonExistent << std::endl;

    // Тест 5: Удаление и очистка
    std::cout << "\nTest 1.5: Remove and clear..." << std::endl;
    blackboard.RemoveValue("PatrolIndex");
    std::cout << "After removing 'PatrolIndex', has key: "
        << (blackboard.HasValue("PatrolIndex") ? "YES" : "NO") << std::endl;

    blackboard.DebugPrint();

    blackboard.Clear();
    std::cout << "After Clear(), size: " << blackboard.GetSize() << std::endl;

    std::cout << "\n✓ Blackboard tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Тестирование BTNode
// ----------------------------------------------------------------------------
static void TestBTNode() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 2: BTNode Lifecycle" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Blackboard blackboard;

    // Тест 1: Узел, возвращающий Success
    std::cout << "Test 2.1: Node returning Success..." << std::endl;
    TestNode successNode("SuccessNode", BTNodeState::Success);
    BTNodeState state = successNode.Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Final state: " << BTNodeStateToString(state) << std::endl;
    std::cout << "Is still running: " << (successNode.IsRunning() ? "YES" : "NO") << std::endl;

    // Тест 2: Узел, возвращающий Running
    std::cout << "\nTest 2.2: Node returning Running (multiple frames)..." << std::endl;
    TestNode runningNode("RunningNode", BTNodeState::Running);

    std::cout << "Frame 1:" << std::endl;
    state = runningNode.Execute(nullptr, &blackboard, 0.016f);
    std::cout << "State: " << BTNodeStateToString(state) << std::endl;
    std::cout << "Is running: " << (runningNode.IsRunning() ? "YES" : "NO") << std::endl;

    std::cout << "\nFrame 2:" << std::endl;
    state = runningNode.Execute(nullptr, &blackboard, 0.016f);
    std::cout << "State: " << BTNodeStateToString(state) << std::endl;

    std::cout << "\nFrame 3 (manual change to Success):" << std::endl;
    // Здесь мы видим, что OnEnter вызывается только один раз
    // OnExit вызывается только когда состояние != Running

    // Тест 3: Прерывание узла
    std::cout << "\nTest 2.3: Aborting a running node..." << std::endl;
    TestNode abortNode("AbortNode", BTNodeState::Running);
    abortNode.Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Before abort - Is running: " << (abortNode.IsRunning() ? "YES" : "NO") << std::endl;
    abortNode.Abort(nullptr, &blackboard);
    std::cout << "After abort - Is running: " << (abortNode.IsRunning() ? "YES" : "NO") << std::endl;

    // Тест 4: Reset узла
    std::cout << "\nTest 2.4: Resetting a node..." << std::endl;
    TestNode resetNode("ResetNode", BTNodeState::Running);
    resetNode.Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Before reset - Is running: " << (resetNode.IsRunning() ? "YES" : "NO") << std::endl;
    resetNode.Reset();
    std::cout << "After reset - Is running: " << (resetNode.IsRunning() ? "YES" : "NO") << std::endl;

    std::cout << "\n✓ BTNode tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Тестирование enum состояний
// ----------------------------------------------------------------------------
static void TestEnums() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 3: Enum States" << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::cout << "Testing BTNodeStateToString():" << std::endl;
    std::cout << "Success -> " << BTNodeStateToString(BTNodeState::Success) << std::endl;
    std::cout << "Failure -> " << BTNodeStateToString(BTNodeState::Failure) << std::endl;
    std::cout << "Running -> " << BTNodeStateToString(BTNodeState::Running) << std::endl;
    std::cout << "Aborted -> " << BTNodeStateToString(BTNodeState::Aborted) << std::endl;

    std::cout << "\n✓ Enum tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Главная функция тестирования (теперь в namespace AI)
// ----------------------------------------------------------------------------
namespace AI {
    void RunStage1Tests() {
        std::cout << "\n" << std::endl;
        std::cout << "############################################" << std::endl;
        std::cout << "#  AI SYSTEM - STAGE 1 TESTS             #" << std::endl;
        std::cout << "#  Testing: Blackboard, BTNode, Enums    #" << std::endl;
        std::cout << "############################################" << std::endl;

        TestEnums();
        TestBlackboard();
        TestBTNode();

        std::cout << "\n############################################" << std::endl;
        std::cout << "#  ALL STAGE 1 TESTS COMPLETED!          #" << std::endl;
        std::cout << "############################################\n" << std::endl;
    }
}