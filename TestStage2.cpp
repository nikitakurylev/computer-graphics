// ============================================================================
// TestStage2.cpp
// Тестирование узлов Behaviour Tree (Этап 2)
// ============================================================================

#include "TestStage2.h"
#include <iostream>
#include "BTNodeState.h"
#include "Blackboard.h"
#include "BTSequence.h"
#include "BTSelector.h"
#include "BTParallel.h"
#include "BTInverter.h"
#include "BTRepeat.h"
#include "BTRetryUntilSuccess.h"
#include "BTAction.h"
#include "BTCondition.h"

using namespace AI;

// ----------------------------------------------------------------------------
// Вспомогательные функции для создания тестовых узлов
// ----------------------------------------------------------------------------

// Действие, которое всегда успешно
static BTNodePtr CreateSuccessAction(const std::string& name) {
    return std::make_shared<BTLambdaAction>(name,
        [name](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  [" << name << "] Executed - SUCCESS" << std::endl;
            return BTNodeState::Success;
        }
    );
}

// Действие, которое всегда проваливается
static BTNodePtr CreateFailureAction(const std::string& name) {
    return std::make_shared<BTLambdaAction>(name,
        [name](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  [" << name << "] Executed - FAILURE" << std::endl;
            return BTNodeState::Failure;
        }
    );
}

// Действие, которое Running N кадров, потом Success
static BTNodePtr CreateRunningAction(const std::string& name, int framesToRun) {
    return std::make_shared<BTLambdaAction>(name,
        [name, framesToRun](GameObject* go, Blackboard* bb, float dt) {
            int counter = bb->GetValue<int>(name + "_counter", 0);
            counter++;
            bb->SetValue(name + "_counter", counter);

            std::cout << "  [" << name << "] Frame " << counter << "/" << framesToRun;

            if (counter >= framesToRun) {
                std::cout << " - SUCCESS" << std::endl;
                bb->RemoveValue(name + "_counter");
                return BTNodeState::Success;
            }
            else {
                std::cout << " - RUNNING" << std::endl;
                return BTNodeState::Running;
            }
        }
    );
}

// Условие, проверяющее значение в blackboard
static BTNodePtr CreateCondition(const std::string& name, const std::string& key, float threshold) {
    return std::make_shared<BTLambdaCondition>(name,
        [name, key, threshold](GameObject* go, Blackboard* bb) {
            float value = bb->GetValue<float>(key, 0.0f);
            bool result = value > threshold;
            std::cout << "  [" << name << "] Check " << key << " > " << threshold
                << " (value=" << value << ") - "
                << (result ? "SUCCESS" : "FAILURE") << std::endl;
            return result ? BTNodeState::Success : BTNodeState::Failure;
        }
    );
}

// ----------------------------------------------------------------------------
// Тест 1: BTSequence
// ----------------------------------------------------------------------------
static void TestSequence() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 1: BTSequence" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Blackboard blackboard;

    // Тест 1.1: Все дети успешны
    std::cout << "Test 1.1: All children succeed..." << std::endl;
    auto sequence1 = std::make_shared<BTSequence>("Seq_AllSuccess");
    sequence1->AddChild(CreateSuccessAction("Action1"));
    sequence1->AddChild(CreateSuccessAction("Action2"));
    sequence1->AddChild(CreateSuccessAction("Action3"));

    BTNodeState result = sequence1->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << std::endl;
    std::cout << "Expected: Success\n" << std::endl;

    // Тест 1.2: Один ребенок проваливается
    std::cout << "Test 1.2: One child fails..." << std::endl;
    auto sequence2 = std::make_shared<BTSequence>("Seq_OneFails");
    sequence2->AddChild(CreateSuccessAction("Action1"));
    sequence2->AddChild(CreateFailureAction("Action2_FAIL"));
    sequence2->AddChild(CreateSuccessAction("Action3_NotExecuted"));

    result = sequence2->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << std::endl;
    std::cout << "Expected: Failure (Action3 should NOT execute)\n" << std::endl;

    // Тест 1.3: Один ребенок Running
    std::cout << "Test 1.3: One child running..." << std::endl;
    auto sequence3 = std::make_shared<BTSequence>("Seq_OneRunning");
    sequence3->AddChild(CreateSuccessAction("Action1"));
    sequence3->AddChild(CreateRunningAction("Action2_Running", 3));
    sequence3->AddChild(CreateSuccessAction("Action3"));

    std::cout << "Frame 1:" << std::endl;
    result = sequence3->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << "\n" << std::endl;

    std::cout << "Frame 2:" << std::endl;
    result = sequence3->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << "\n" << std::endl;

    std::cout << "Frame 3:" << std::endl;
    result = sequence3->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << std::endl;
    std::cout << "Expected: Success (after 3 frames)\n" << std::endl;

    std::cout << "✓ Sequence tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Тест 2: BTSelector
// ----------------------------------------------------------------------------
static void TestSelector() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 2: BTSelector" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Blackboard blackboard;

    // Тест 2.1: Первый ребенок успешен
    std::cout << "Test 2.1: First child succeeds..." << std::endl;
    auto selector1 = std::make_shared<BTSelector>("Sel_FirstSuccess");
    selector1->AddChild(CreateSuccessAction("Action1"));
    selector1->AddChild(CreateSuccessAction("Action2_NotExecuted"));
    selector1->AddChild(CreateSuccessAction("Action3_NotExecuted"));

    BTNodeState result = selector1->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << std::endl;
    std::cout << "Expected: Success (Action2 & 3 should NOT execute)\n" << std::endl;

    // Тест 2.2: Все дети проваливаются
    std::cout << "Test 2.2: All children fail..." << std::endl;
    auto selector2 = std::make_shared<BTSelector>("Sel_AllFail");
    selector2->AddChild(CreateFailureAction("Action1"));
    selector2->AddChild(CreateFailureAction("Action2"));
    selector2->AddChild(CreateFailureAction("Action3"));

    result = selector2->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << std::endl;
    std::cout << "Expected: Failure\n" << std::endl;

    // Тест 2.3: Третий ребенок успешен
    std::cout << "Test 2.3: Third child succeeds..." << std::endl;
    auto selector3 = std::make_shared<BTSelector>("Sel_ThirdSuccess");
    selector3->AddChild(CreateFailureAction("Action1"));
    selector3->AddChild(CreateFailureAction("Action2"));
    selector3->AddChild(CreateSuccessAction("Action3"));

    result = selector3->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << std::endl;
    std::cout << "Expected: Success\n" << std::endl;

    std::cout << "✓ Selector tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Тест 3: BTParallel
// ----------------------------------------------------------------------------
static void TestParallel() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 3: BTParallel" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Blackboard blackboard;

    // Тест 3.1: RequireAll - все успешны
    std::cout << "Test 3.1: RequireAll - all succeed..." << std::endl;
    auto parallel1 = std::make_shared<BTParallel>(ParallelPolicy::RequireAll, "Par_AllSuccess");
    parallel1->AddChild(CreateSuccessAction("Action1"));
    parallel1->AddChild(CreateSuccessAction("Action2"));
    parallel1->AddChild(CreateSuccessAction("Action3"));

    BTNodeState result = parallel1->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << std::endl;
    std::cout << "Expected: Success\n" << std::endl;

    // Тест 3.2: RequireAll - один проваливается
    std::cout << "Test 3.2: RequireAll - one fails..." << std::endl;
    auto parallel2 = std::make_shared<BTParallel>(ParallelPolicy::RequireAll, "Par_OneFails");
    parallel2->AddChild(CreateSuccessAction("Action1"));
    parallel2->AddChild(CreateFailureAction("Action2"));
    parallel2->AddChild(CreateSuccessAction("Action3"));

    result = parallel2->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << std::endl;
    std::cout << "Expected: Failure\n" << std::endl;

    // Тест 3.3: RequireOne - один успешен
    std::cout << "Test 3.3: RequireOne - one succeeds..." << std::endl;
    auto parallel3 = std::make_shared<BTParallel>(ParallelPolicy::RequireOne, "Par_OneSuccess");
    parallel3->AddChild(CreateFailureAction("Action1"));
    parallel3->AddChild(CreateSuccessAction("Action2"));
    parallel3->AddChild(CreateFailureAction("Action3"));

    result = parallel3->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << std::endl;
    std::cout << "Expected: Success\n" << std::endl;

    std::cout << "✓ Parallel tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Тест 4: Декораторы
// ----------------------------------------------------------------------------
static void TestDecorators() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 4: Decorators" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Blackboard blackboard;

    // Тест 4.1: Inverter
    std::cout << "Test 4.1: Inverter..." << std::endl;
    auto inverter1 = std::make_shared<BTInverter>("Inv_Success");
    inverter1->SetChild(CreateSuccessAction("Action"));
    BTNodeState result = inverter1->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << " (Expected: Failure)\n" << std::endl;

    auto inverter2 = std::make_shared<BTInverter>("Inv_Failure");
    inverter2->SetChild(CreateFailureAction("Action"));
    result = inverter2->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << " (Expected: Success)\n" << std::endl;

    // Тест 4.2: Repeat
    std::cout << "Test 4.2: Repeat (3 times)..." << std::endl;
    auto repeat = std::make_shared<BTRepeat>(3, "Repeat3");
    repeat->SetChild(CreateSuccessAction("Action"));

    result = repeat->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << " (Expected: Success)\n" << std::endl;

    // Тест 4.3: RetryUntilSuccess
    std::cout << "Test 4.3: RetryUntilSuccess..." << std::endl;
    blackboard.SetValue("attempt", 0);
    auto retry = std::make_shared<BTRetryUntilSuccess>(5, "Retry");
    retry->SetChild(std::make_shared<BTLambdaAction>("FailTwice",
        [](GameObject* go, Blackboard* bb, float dt) {
            int attempt = bb->GetValue<int>("attempt", 0);
            attempt++;
            bb->SetValue("attempt", attempt);
            std::cout << "  Attempt " << attempt;
            if (attempt < 3) {
                std::cout << " - FAILURE" << std::endl;
                return BTNodeState::Failure;
            }
            std::cout << " - SUCCESS" << std::endl;
            return BTNodeState::Success;
        }
    ));

    result = retry->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << " (Expected: Success after 3 attempts)\n" << std::endl;

    std::cout << "✓ Decorator tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Тест 5: Комплексное дерево
// ----------------------------------------------------------------------------
static void TestComplexTree() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 5: Complex Tree" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Blackboard blackboard;
    blackboard.SetValue("Health", 75.0f);
    blackboard.SetValue("Ammo", 10.0f);

    // Создаем сложное дерево:
    // Selector {
    //     Sequence {
    //         CheckHealth > 50
    //         CheckAmmo > 5
    //         Attack
    //     }
    //     Flee
    // }

    auto root = std::make_shared<BTSelector>("Root");

    auto combatSequence = std::make_shared<BTSequence>("CombatSequence");
    combatSequence->AddChild(CreateCondition("CheckHealth", "Health", 50.0f));
    combatSequence->AddChild(CreateCondition("CheckAmmo", "Ammo", 5.0f));
    combatSequence->AddChild(CreateSuccessAction("Attack"));

    root->AddChild(combatSequence);
    root->AddChild(CreateSuccessAction("Flee"));

    std::cout << "Scenario 1: Health=75, Ammo=10 (should attack)" << std::endl;
    BTNodeState result = root->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << "\n" << std::endl;

    root->Reset();
    blackboard.SetValue("Health", 30.0f);

    std::cout << "Scenario 2: Health=30, Ammo=10 (should flee)" << std::endl;
    result = root->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << "\n" << std::endl;

    std::cout << "✓ Complex tree test completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Главная функция тестирования
// ----------------------------------------------------------------------------
namespace AI {
    void RunStage2Tests() {
        std::cout << "\n" << std::endl;
        std::cout << "############################################" << std::endl;
        std::cout << "#  AI SYSTEM - STAGE 2 TESTS             #" << std::endl;
        std::cout << "#  Testing: BT Nodes                     #" << std::endl;
        std::cout << "############################################" << std::endl;

        TestSequence();
        TestSelector();
        TestParallel();
        TestDecorators();
        TestComplexTree();

        std::cout << "\n############################################" << std::endl;
        std::cout << "#  ALL STAGE 2 TESTS COMPLETED!          #" << std::endl;
        std::cout << "############################################\n" << std::endl;
    }
}