// ============================================================================
// TestStage3.cpp
// Тестирование Utility AI и AIComponent (Этап 3)
// ============================================================================

#include "TestStage3.h"
#include <iostream>
#include "Blackboard.h"
#include "UtilityConsideration.h"
#include "UtilityAction.h"
#include "UtilitySelector.h"
#include "AIComponent.h"
#include "BTSequence.h"
#include "BTSelector.h"
#include "GameObject.h"
#include "Game.h"
#include "BTCondition.h"

using namespace AI;

// ----------------------------------------------------------------------------
// Тест 1: UtilityConsideration и ResponseCurve
// ----------------------------------------------------------------------------
static void TestConsiderations() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 1: UtilityConsideration" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Blackboard blackboard;
    blackboard.SetValue("Health", 75.0f);
    blackboard.SetValue("Distance", 15.0f);

    // Тест 1.1: Простой Consideration
    std::cout << "Test 1.1: Health Consideration (Linear)..." << std::endl;
    auto healthConsideration = std::make_shared<LambdaConsideration>("Health",
        [](GameObject* go, Blackboard* bb) {
            float health = bb->GetValue<float>("Health", 0.0f);
            return health / 100.0f; // Нормализуем 0-1
        }
    );

    float utility = healthConsideration->Evaluate(nullptr, &blackboard);
    std::cout << "Health=75 -> Utility=" << utility << " (expected ~0.75)\n" << std::endl;

    // Тест 1.2: Consideration с квадратичной кривой
    std::cout << "Test 1.2: Same with Quadratic curve..." << std::endl;
    healthConsideration->SetResponseCurve(ResponseCurve::Quadratic);
    utility = healthConsideration->Evaluate(nullptr, &blackboard);
    std::cout << "Health=75 with Quadratic -> Utility=" << utility
        << " (expected ~0.56, т.к. 0.75^2)\n" << std::endl;

    // Тест 1.3: Consideration с весом
    std::cout << "Test 1.3: With weight=0.5..." << std::endl;
    healthConsideration->SetResponseCurve(ResponseCurve::Linear);
    healthConsideration->SetWeight(0.5f);
    utility = healthConsideration->Evaluate(nullptr, &blackboard);
    std::cout << "Health=75 with Weight=0.5 -> Utility=" << utility
        << " (expected ~0.375, т.к. 0.75 * 0.5)\n" << std::endl;

    // Тест 1.4: Разные кривые
    std::cout << "Test 1.4: Testing different curves (input=0.5)..." << std::endl;
    blackboard.SetValue("TestValue", 50.0f);
    auto testConsideration = std::make_shared<LambdaConsideration>("Test",
        [](GameObject* go, Blackboard* bb) {
            return bb->GetValue<float>("TestValue", 0.0f) / 100.0f;
        }
    );

    ResponseCurve curves[] = {
        ResponseCurve::Linear,
        ResponseCurve::Quadratic,
        ResponseCurve::InverseQuadratic,
        ResponseCurve::Sigmoid
    };
    const char* curveNames[] = {
        "Linear",
        "Quadratic",
        "InverseQuadratic",
        "Sigmoid"
    };

    for (int i = 0; i < 4; i++) {
        testConsideration->SetResponseCurve(curves[i]);
        testConsideration->SetWeight(1.0f);
        float result = testConsideration->Evaluate(nullptr, &blackboard);
        std::cout << "  " << curveNames[i] << ": " << result << std::endl;
    }

    std::cout << "\n✓ Consideration tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Тест 2: UtilityAction
// ----------------------------------------------------------------------------
static void TestUtilityActions() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 2: UtilityAction" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Blackboard blackboard;
    blackboard.SetValue("Health", 80.0f);
    blackboard.SetValue("Ammo", 50.0f);
    blackboard.SetValue("Distance", 10.0f);

    // Создаём действие "Attack" с несколькими факторами
    auto attackAction = std::make_shared<LambdaUtilityAction>("Attack",
        [](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  [Attack] Executing attack!" << std::endl;
            return BTNodeState::Success;
        }
    );

    // Добавляем факторы для оценки
    auto healthFactor = std::make_shared<LambdaConsideration>("HealthFactor",
        [](GameObject* go, Blackboard* bb) {
            return bb->GetValue<float>("Health", 0.0f) / 100.0f;
        }
    );

    auto ammoFactor = std::make_shared<LambdaConsideration>("AmmoFactor",
        [](GameObject* go, Blackboard* bb) {
            return bb->GetValue<float>("Ammo", 0.0f) / 100.0f;
        }
    );

    auto distanceFactor = std::make_shared<LambdaConsideration>("DistanceFactor",
        [](GameObject* go, Blackboard* bb) {
            float distance = bb->GetValue<float>("Distance", 100.0f);
            // Инвертируем: ближе = лучше
            return 1.0f - (distance / 100.0f);
        }
    );

    attackAction->AddConsideration(healthFactor);
    attackAction->AddConsideration(ammoFactor);
    attackAction->AddConsideration(distanceFactor);

    // Тест 2.1: Расчёт полезности
    std::cout << "Test 2.1: Calculate utility for Attack action..." << std::endl;
    std::cout << "State: Health=80, Ammo=50, Distance=10" << std::endl;
    float utility = attackAction->CalculateUtility(nullptr, &blackboard);
    std::cout << "Attack Utility: " << utility << "\n" << std::endl;

    // Тест 2.2: Изменение параметров
    std::cout << "Test 2.2: Change parameters..." << std::endl;
    blackboard.SetValue("Health", 20.0f);
    blackboard.SetValue("Distance", 80.0f);
    std::cout << "State: Health=20, Ammo=50, Distance=80" << std::endl;
    utility = attackAction->CalculateUtility(nullptr, &blackboard);
    std::cout << "Attack Utility: " << utility << " (должна быть ниже)\n" << std::endl;

    // Тест 2.3: Базовая полезность
    std::cout << "Test 2.3: Base utility modifier..." << std::endl;
    attackAction->SetBaseUtility(0.3f);
    utility = attackAction->CalculateUtility(nullptr, &blackboard);
    std::cout << "Attack Utility with +0.3 base: " << utility << "\n" << std::endl;

    std::cout << "✓ UtilityAction tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Тест 3: UtilitySelector
// ----------------------------------------------------------------------------
static void TestUtilitySelector() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 3: UtilitySelector" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Blackboard blackboard;
    blackboard.SetValue("Health", 75.0f);
    blackboard.SetValue("EnemyDistance", 5.0f);

    // Создаём несколько действий с разной полезностью
    auto attackAction = std::make_shared<LambdaUtilityAction>("Attack",
        [](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  → Executing: ATTACK" << std::endl;
            return BTNodeState::Success;
        }
    );

    auto fleeAction = std::make_shared<LambdaUtilityAction>("Flee",
        [](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  → Executing: FLEE" << std::endl;
            return BTNodeState::Success;
        }
    );

    auto healAction = std::make_shared<LambdaUtilityAction>("Heal",
        [](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  → Executing: HEAL" << std::endl;
            return BTNodeState::Success;
        }
    );

    // Настраиваем факторы для Attack (хорошо когда здоров и враг близко)
    attackAction->AddConsideration(std::make_shared<LambdaConsideration>("HealthForAttack",
        [](GameObject* go, Blackboard* bb) {
            return bb->GetValue<float>("Health", 0.0f) / 100.0f;
        }
    ));
    attackAction->AddConsideration(std::make_shared<LambdaConsideration>("DistanceForAttack",
        [](GameObject* go, Blackboard* bb) {
            float dist = bb->GetValue<float>("EnemyDistance", 100.0f);
            return 1.0f - (dist / 20.0f); // Инвертируем
        }
    ));

    // Настраиваем факторы для Flee (хорошо когда здоровье низкое)
    fleeAction->AddConsideration(std::make_shared<LambdaConsideration>("HealthForFlee",
        [](GameObject* go, Blackboard* bb) {
            float health = bb->GetValue<float>("Health", 100.0f);
            return 1.0f - (health / 100.0f); // Инвертируем: меньше здоровья = выше полезность
        }
    ));

    // Настраиваем факторы для Heal (хорошо когда здоровье среднее)
    healAction->AddConsideration(std::make_shared<LambdaConsideration>("HealthForHeal",
        [](GameObject* go, Blackboard* bb) {
            float health = bb->GetValue<float>("Health", 100.0f);
            // Оптимально при 30-50 здоровья
            if (health < 30.0f) return 0.3f;
            if (health > 70.0f) return 0.1f;
            return 0.8f;
        }
    ));

    // Создаём селектор
    auto utilitySelector = std::make_shared<UtilitySelector>("ActionSelector");
    utilitySelector->AddChild(attackAction);
    utilitySelector->AddChild(fleeAction);
    utilitySelector->AddChild(healAction);

    // Тест 3.1: Высокое здоровье, враг близко
    std::cout << "Test 3.1: Health=75, EnemyDistance=5..." << std::endl;
    BTNodeState result = utilitySelector->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << "\n" << std::endl;
    utilitySelector->Reset();

    // Тест 3.2: Низкое здоровье
    std::cout << "Test 3.2: Health=15, EnemyDistance=5..." << std::endl;
    blackboard.SetValue("Health", 15.0f);
    result = utilitySelector->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << "\n" << std::endl;
    utilitySelector->Reset();

    // Тест 3.3: Среднее здоровье
    std::cout << "Test 3.3: Health=45, EnemyDistance=15..." << std::endl;
    blackboard.SetValue("Health", 45.0f);
    blackboard.SetValue("EnemyDistance", 15.0f);
    result = utilitySelector->Execute(nullptr, &blackboard, 0.016f);
    std::cout << "Result: " << BTNodeStateToString(result) << "\n" << std::endl;

    std::cout << "✓ UtilitySelector tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Тест 4: AIComponent
// ----------------------------------------------------------------------------
static void TestAIComponent() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 4: AIComponent" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Создаём простое дерево для тестирования
    auto selector = std::make_shared<BTSelector>("RootSelector");

    auto checkHealth = std::make_shared<BTLambdaCondition>("CheckHealth",
        [](GameObject* go, Blackboard* bb) {
            float health = bb->GetValue<float>("Health", 100.0f);
            std::cout << "  [CheckHealth] Health=" << health << std::endl;
            return (health > 50.0f) ? BTNodeState::Success : BTNodeState::Failure;
        }
    );

    auto attackAction = std::make_shared<BTLambdaAction>("Attack",
        [](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  [Attack] Attacking enemy!" << std::endl;
            return BTNodeState::Success;
        }
    );

    auto fleeAction = std::make_shared<BTLambdaAction>("Flee",
        [](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  [Flee] Running away!" << std::endl;
            return BTNodeState::Success;
        }
    );

    auto attackSequence = std::make_shared<BTSequence>("AttackSequence");
    attackSequence->AddChild(checkHealth);
    attackSequence->AddChild(attackAction);

    selector->AddChild(attackSequence);
    selector->AddChild(fleeAction);

    // Тест 4.1: Создание компонента
    std::cout << "Test 4.1: Creating AIComponent..." << std::endl;
    auto aiComponent = new AIComponent();
    aiComponent->SetBehaviourTree(selector);
    aiComponent->GetBlackboard()->SetValue("Health", 75.0f);
    std::cout << "AIComponent created and tree set\n" << std::endl;

    // Тест 4.2: Симуляция Update (здоровье > 50)
    std::cout << "Test 4.2: Simulating Update with Health=75..." << std::endl;
    aiComponent->Start();
    aiComponent->Update(0.016f);
    std::cout << std::endl;

    // Тест 4.3: Изменение данных (здоровье < 50)
    std::cout << "Test 4.3: Changing Health to 30 and updating..." << std::endl;
    aiComponent->GetBlackboard()->SetValue("Health", 30.0f);
    aiComponent->Reset();
    aiComponent->Update(0.016f);
    std::cout << std::endl;

    // Тест 4.4: Включение/выключение
    std::cout << "Test 4.4: Disabling AI..." << std::endl;
    aiComponent->SetEnabled(false);
    aiComponent->Update(0.016f);
    std::cout << "AI was disabled, no action taken\n" << std::endl;

    std::cout << "Test 4.5: Re-enabling AI..." << std::endl;
    aiComponent->SetEnabled(true);
    aiComponent->Update(0.016f);
    std::cout << std::endl;

    delete aiComponent;

    std::cout << "✓ AIComponent tests completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Тест 5: Полная интеграция
// ----------------------------------------------------------------------------
static void TestFullIntegration() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 5: Full Integration" << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::cout << "Test 5.1: Creating complex AI with Utility + BT..." << std::endl;

    // Создаём комплексное дерево с интеграцией Utility AI
    auto root = std::make_shared<BTSequence>("Root");

    // Сначала выбираем тактическое действие через Utility
    auto tacticalSelector = std::make_shared<UtilitySelector>("TacticalActions");

    // Действие 1: Атака ближнего боя
    auto meleeAttack = std::make_shared<LambdaUtilityAction>("MeleeAttack",
        [](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  → Executing: Melee Attack" << std::endl;
            return BTNodeState::Success;
        }
    );
    meleeAttack->AddConsideration(std::make_shared<LambdaConsideration>("MeleeDistance",
        [](GameObject* go, Blackboard* bb) {
            float dist = bb->GetValue<float>("TargetDistance", 100.0f);
            return (dist < 5.0f) ? 1.0f : 0.0f; // Только если очень близко
        }
    ));

    // Действие 2: Дальняя атака
    auto rangedAttack = std::make_shared<LambdaUtilityAction>("RangedAttack",
        [](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  → Executing: Ranged Attack" << std::endl;
            return BTNodeState::Success;
        }
    );
    rangedAttack->AddConsideration(std::make_shared<LambdaConsideration>("RangedDistance",
        [](GameObject* go, Blackboard* bb) {
            float dist = bb->GetValue<float>("TargetDistance", 100.0f);
            float ammo = bb->GetValue<float>("Ammo", 0.0f);
            if (ammo <= 0.0f) return 0.0f;
            return (dist > 5.0f && dist < 20.0f) ? 0.9f : 0.3f;
        }
    ));

    // Действие 3: Преследование
    auto chase = std::make_shared<LambdaUtilityAction>("Chase",
        [](GameObject* go, Blackboard* bb, float dt) {
            std::cout << "  → Executing: Chase" << std::endl;
            return BTNodeState::Success;
        }
    );
    chase->AddConsideration(std::make_shared<LambdaConsideration>("ChaseDistance",
        [](GameObject* go, Blackboard* bb) {
            float dist = bb->GetValue<float>("TargetDistance", 100.0f);
            return (dist > 20.0f) ? 0.8f : 0.2f;
        }
    ));

    tacticalSelector->AddChild(meleeAttack);
    tacticalSelector->AddChild(rangedAttack);
    tacticalSelector->AddChild(chase);

    root->AddChild(tacticalSelector);

    // Создаём AI компонент
    auto aiComponent = new AIComponent();
    aiComponent->SetBehaviourTree(root);

    // Тест разных сценариев
    std::cout << "\nScenario 1: Close range, has ammo" << std::endl;
    aiComponent->GetBlackboard()->SetValue("TargetDistance", 3.0f);
    aiComponent->GetBlackboard()->SetValue("Ammo", 10.0f);
    aiComponent->Update(0.016f);
    aiComponent->Reset();

    std::cout << "\nScenario 2: Medium range, has ammo" << std::endl;
    aiComponent->GetBlackboard()->SetValue("TargetDistance", 12.0f);
    aiComponent->GetBlackboard()->SetValue("Ammo", 10.0f);
    aiComponent->Update(0.016f);
    aiComponent->Reset();

    std::cout << "\nScenario 3: Far range" << std::endl;
    aiComponent->GetBlackboard()->SetValue("TargetDistance", 30.0f);
    aiComponent->GetBlackboard()->SetValue("Ammo", 10.0f);
    aiComponent->Update(0.016f);
    aiComponent->Reset();

    std::cout << "\nScenario 4: Medium range, no ammo" << std::endl;
    aiComponent->GetBlackboard()->SetValue("TargetDistance", 12.0f);
    aiComponent->GetBlackboard()->SetValue("Ammo", 0.0f);
    aiComponent->Update(0.016f);

    delete aiComponent;

    std::cout << "\n✓ Full integration test completed!\n" << std::endl;
}

// ----------------------------------------------------------------------------
// Главная функция тестирования
// ----------------------------------------------------------------------------
namespace AI {
    void RunStage3Tests() {
        std::cout << "\n" << std::endl;
        std::cout << "############################################" << std::endl;
        std::cout << "#  AI SYSTEM - STAGE 3 TESTS             #" << std::endl;
        std::cout << "#  Testing: Utility AI & AIComponent     #" << std::endl;
        std::cout << "############################################" << std::endl;

        TestConsiderations();
        TestUtilityActions();
        TestUtilitySelector();
        TestAIComponent();
        TestFullIntegration();

        std::cout << "\n############################################" << std::endl;
        std::cout << "#  ALL STAGE 3 TESTS COMPLETED!          #" << std::endl;
        std::cout << "############################################\n" << std::endl;
    }
}