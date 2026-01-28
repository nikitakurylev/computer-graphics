#pragma once
#include "BTCompositeNode.h"
#include "UtilityAction.h"
#include <algorithm>

// ============================================================================
// UtilitySelector.h
// Узел BT, который выбирает действие на основе полезности
// ============================================================================

namespace AI {

    // UtilitySelector - выбирает ребенка с максимальной полезностью
    class UtilitySelector : public BTCompositeNode {
    public:
        UtilitySelector(const std::string& name = "UtilitySelector")
            : BTCompositeNode(name), selectedChildIndex(-1), minUtilityThreshold(0.0f) {
        }

        // Установить минимальный порог полезности
        // Действия с полезностью ниже этого порога не будут рассматриваться
        void SetMinUtilityThreshold(float threshold) {
            minUtilityThreshold = threshold;
        }

    protected:

        // При входе - оцениваем и выбираем лучшее действие
        void OnEnter(GameObject* gameObject, Blackboard* blackboard) override {
            selectedChildIndex = -1;

            if (children.empty()) {
                return;
            }

            // Оцениваем полезность каждого ребенка
            float bestUtility = minUtilityThreshold;
            int bestIndex = -1;

            for (size_t i = 0; i < children.size(); i++) {
                // Пытаемся привести к UtilityAction
                auto utilityAction = std::dynamic_pointer_cast<UtilityAction>(children[i]);

                float utility = 0.0f;
                if (utilityAction) {
                    // Это UtilityAction - рассчитываем полезность
                    utility = utilityAction->CalculateUtility(gameObject, blackboard);
                }
                else {
                    // Обычный узел - используем базовую полезность 0.5
                    utility = 0.5f;
                }

#ifdef AI_DEBUG
                std::cout << "[UtilitySelector] Child '" << children[i]->GetName()
                    << "' utility: " << utility << std::endl;
#endif

                // Выбираем лучшее
                if (utility > bestUtility) {
                    bestUtility = utility;
                    bestIndex = static_cast<int>(i);
                }
            }

            selectedChildIndex = bestIndex;

#ifdef AI_DEBUG
            if (selectedChildIndex >= 0) {
                std::cout << "[UtilitySelector] Selected: '"
                    << children[selectedChildIndex]->GetName()
                    << "' with utility " << bestUtility << std::endl;
            }
            else {
                std::cout << "[UtilitySelector] No action selected (all below threshold)"
                    << std::endl;
            }
#endif
        }

        // Выполняем выбранное действие
        BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) override {
            // Нет детей
            if (children.empty()) {
                return BTNodeState::Failure;
            }

            // Ничего не выбрано (все ниже порога)
            if (selectedChildIndex < 0 || selectedChildIndex >= static_cast<int>(children.size())) {
                return BTNodeState::Failure;
            }

            // Выполняем выбранное действие
            return children[selectedChildIndex]->Execute(gameObject, blackboard, deltaTime);
        }

        // При выходе - сбрасываем выбор
        void OnExit(GameObject* gameObject, Blackboard* blackboard) override {
            if (selectedChildIndex >= 0 && selectedChildIndex < static_cast<int>(children.size())) {
                children[selectedChildIndex]->Reset();
            }
            selectedChildIndex = -1;
        }

        // При прерывании - прерываем выбранного ребенка
        void OnAbort(GameObject* gameObject, Blackboard* blackboard) override {
            if (selectedChildIndex >= 0 && selectedChildIndex < static_cast<int>(children.size())) {
                children[selectedChildIndex]->Abort(gameObject, blackboard);
            }
        }

    private:
        int selectedChildIndex;      // Индекс выбранного действия
        float minUtilityThreshold;   // Минимальный порог полезности
    };

}