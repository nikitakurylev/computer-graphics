#pragma once
#include "BTNode.h"
#include <functional>

// ============================================================================
// BTAction.h
// Базовый класс для листовых узлов-действий
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // BTAction - листовой узел, выполняющий конкретное действие
    // 
    // Action узлы - это "листья" дерева, которые выполняют реальные действия:
    // - Движение
    // - Атака
    // - Проигрывание анимации
    // - Изменение состояния
    // 
    // Создавайте наследников от этого класса для своих действий
    // ------------------------------------------------------------------------
    class BTAction : public BTNode {
    public:
        BTAction(const std::string& name = "Action")
            : BTNode(name) {
        }

        virtual ~BTAction() = default;

    protected:
        // Переопределите Tick() в наследниках для реализации действия
        // Пример см. в BTLambdaAction
    };

    // ------------------------------------------------------------------------
    // BTLambdaAction - действие через лямбда-функцию (для быстрого прототипирования)
    // 
    // Позволяет создавать действия без создания отдельных классов
    // 
    // ПРИМЕР ИСПОЛЬЗОВАНИЯ:
    // auto moveAction = std::make_shared<BTLambdaAction>("Move", 
    //     [](GameObject* go, Blackboard* bb, float dt) {
    //         go->GetTransform()->position.x += 1.0f * dt;
    //         return BTNodeState::Success;
    //     }
    // );
    // ------------------------------------------------------------------------
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

} // namespace AI