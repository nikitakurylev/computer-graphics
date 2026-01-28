#pragma once
#include <vector>
#include <string>
#include <memory>
#include "UtilityConsideration.h"
#include "BTAction.h"

// ============================================================================
// UtilityAction.h
// Действие с автоматическим расчётом полезности
// ============================================================================

namespace AI {

    class UtilityAction : public BTAction {
    public:
        UtilityAction(const std::string& name = "UtilityAction")
            : BTAction(name), baseUtility(0.0f) {
        }

        virtual ~UtilityAction() = default;

        // Добавить фактор для оценки полезности
        void AddConsideration(ConsiderationPtr consideration) {
            considerations.push_back(consideration);
        }

        // Рассчитать полезность этого действия
        // Возвращает значение от 0.0 (не полезно) до 1.0+ (очень полезно)
        float CalculateUtility(GameObject* gameObject, Blackboard* blackboard) {
            if (considerations.empty()) {
                return baseUtility;
            }

            float sum = 0.0f;
            float product = 1.0f;

            // Рассчитываем среднее и произведение всех факторов
            for (auto& consideration : considerations) {
                float value = consideration->Evaluate(gameObject, blackboard);
                sum += value;
                product *= value;
            }

            float average = sum / considerations.size();

            // Компенсирующая формула: учитывает оба значения
            // Если хотя бы один фактор = 0, итог близок к 0
            // Если все факторы высокие, итог высокий
            float modificationFactor = 1.0f - (1.0f / considerations.size());
            float utility = average + (modificationFactor * (product - average));

            return utility + baseUtility;
        }

        // Установить базовую полезность (бонус/штраф)
        // Полезно для приоритизации определённых действий
        void SetBaseUtility(float base) {
            baseUtility = base;
        }


        // Получить количество факторов
        size_t GetConsiderationCount() const {
            return considerations.size();
        }

    protected:
        std::vector<ConsiderationPtr> considerations;
        float baseUtility;  // Базовая полезность (модификатор)
    };

    // Умный указатель на UtilityAction
    using UtilityActionPtr = std::shared_ptr<UtilityAction>;

    // LambdaUtilityAction - UtilityAction с лямбда-функцией для выполнения
    class LambdaUtilityAction : public UtilityAction {
    public:
        using ActionFunc = std::function<BTNodeState(GameObject*, Blackboard*, float)>;

        LambdaUtilityAction(const std::string& name, ActionFunc func)
            : UtilityAction(name), actionFunc(func) {
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