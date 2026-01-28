#pragma once
#include "BTNode.h"
#include <functional>

// ============================================================================
// BTCondition.h
// Базовый класс для листовых узлов-условий (проверок)
// ============================================================================

namespace AI {
    class BTCondition : public BTNode {
    public:
        BTCondition(const std::string& name = "Condition")
            : BTNode(name) {
        }

        virtual ~BTCondition() = default;

    protected:

    };

    // BTLambdaCondition - условие через лямбда-функцию
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

}