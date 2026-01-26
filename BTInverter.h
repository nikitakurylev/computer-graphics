#pragma once
#include "BTDecorator.h"

// ============================================================================
// BTInverter.h
// Декоратор "Инвертор" - инвертирует результат ребенка
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // BTInverter - инвертирует результат дочернего узла
    // 
    // ЛОГИКА:
    // - Success → Failure
    // - Failure → Success
    // - Running → Running (не меняется)
    // - Aborted → Aborted (не меняется)
    // 
    // АНАЛОГ: логическое НЕ (NOT)
    // 
    // ПРИМЕР ИСПОЛЬЗОВАНИЯ:
    // Inverter {
    //     IsHealthLow()  ← Проверка "Здоровье низкое?"
    // }
    // Вернет Success, если здоровье НЕ низкое
    // 
    // Полезно для создания условий типа "если НЕ ..."
    // ------------------------------------------------------------------------
    class BTInverter : public BTDecorator {
    public:
        BTInverter(const std::string& name = "Inverter")
            : BTDecorator(name) {
        }

    protected:
        // --------------------------------------------------------------------
        // Выполняем ребенка и инвертируем результат
        // --------------------------------------------------------------------
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

} // namespace AI