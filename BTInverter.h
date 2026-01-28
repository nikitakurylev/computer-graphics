#pragma once
#include "BTDecorator.h"

// ============================================================================
// BTInverter.h
// Декоратор "Инвертор" - инвертирует результат ребенка
// ============================================================================

namespace AI {

    class BTInverter : public BTDecorator {
    public:
        BTInverter(const std::string& name = "Inverter")
            : BTDecorator(name) {
        }

    protected:
        // Выполняем ребенка и инвертируем результат
        BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) override {
            // Проверка наличия ребенка
            if (!child) {
                return BTNodeState::Failure;
            }

            // Выполняем ребенка
            BTNodeState childState = child->Execute(gameObject, blackboard, deltaTime);

            // Инвертируем результат
            switch (childState) {
            case BTNodeState::Success:
                return BTNodeState::Failure;

            case BTNodeState::Failure:
                return BTNodeState::Success;

            case BTNodeState::Running:
                // Running не инвертируется
                return BTNodeState::Running;

            case BTNodeState::Aborted:
                // Aborted не инвертируется
                return BTNodeState::Aborted;

            default:
                return BTNodeState::Failure;
            }
        }
    };

}