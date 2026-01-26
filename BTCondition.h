#pragma once
#include "BTNode.h"
#include <functional>

// ============================================================================
// BTCondition.h
// Базовый класс для листовых узлов-условий (проверок)
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // BTCondition - листовой узел, проверяющий условие
    // 
    // Condition узлы - это проверки, которые возвращают Success/Failure:
    // - Проверка здоровья
    // - Проверка расстояния до цели
    // - Проверка наличия патронов
    // - Проверка видимости врага
    // 
    // Обычно используются в Selector/Sequence для принятия решений
    // ------------------------------------------------------------------------
    class BTCondition : public BTNode {
    public:
        BTCondition(const std::string& name = "Condition")
            : BTNode(name) {
        }

        virtual ~BTCondition() = default;

    protected:
        // Переопределите Tick() в наследниках для реализации проверки
        // Должен возвращать только Success или Failure (НЕ Running!)
    };

    // ------------------------------------------------------------------------
    // BTLambdaCondition - условие через лямбда-функцию
    // 
    // Позволяет создавать условия без создания отдельных классов
    // 
    // ПРИМЕР ИСПОЛЬЗОВАНИЯ:
    // auto healthCheck = std::make_shared<BTLambdaCondition>("CheckHealth",
    //     [](GameObject* go, Blackboard* bb) {
    //         float health = bb->GetValue<float>("Health", 100.0f);
    //         return (health > 50.0f) ? BTNodeState::Success : BTNodeState::Failure;
    //     }
    // );
    // ------------------------------------------------------------------------
    class BTLambdaCondition : public BTCondition {
    public:
        using ConditionFunc = std::function<BTNodeState(GameObject*, Blackboard*)>;

        BTLambdaCondition(const std::string& name, ConditionFunc func)
            : BTCondition(name), conditionFunc(func) {
        }

    protected:
        BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) override {
            if (conditionFunc) {
                return conditionFunc(gameObject, blackboard);
            }
            return BTNodeState::Failure;
        }

    private:
        ConditionFunc conditionFunc;
    };

} // namespace AI