#pragma once
#include "BTNode.h"
#include <functional>

// ============================================================================
// BTAction.h
// Базовый класс для листовых узлов-действий
// ============================================================================

namespace AI {

    class BTAction : public BTNode {
    public:
        BTAction(const std::string& name = "Action")
            : BTNode(name) {
        }

        virtual ~BTAction() = default;

    protected:

    };

    // BTLambdaAction - действие через лямбда-функцию
    class BTLambdaAction : public BTAction {
    public:
        using ActionFunc = std::function<BTNodeState(GameObject*, Blackboard*, float)>;

        BTLambdaAction(const std::string& name, ActionFunc func)
            : BTAction(name), actionFunc(func) {
        }

    protected:
        BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) override {
            if (actionFunc) {
                return actionFunc(gameObject, blackboard, deltaTime);
            }
            return BTNodeState::Failure;
        }

    private:
        ActionFunc actionFunc;
    };

}