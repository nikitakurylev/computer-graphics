#pragma once
#include "BTCompositeNode.h"

// ============================================================================
// BTSelector.h
// Узел выбирает первого успешного ребенка
// ============================================================================

namespace AI {


    class BTSelector : public BTCompositeNode {
    public:
        BTSelector(const std::string& name = "Selector")
            : BTCompositeNode(name) {
        }

    protected:

        // При входе - начинаем с первого ребенка
        void OnEnter(GameObject* gameObject, Blackboard* blackboard) override {
            currentChildIndex = 0;
        }

        // Выполнение селектора
        BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) override {
            // Проверка на отсутствие детей
            if (children.empty()) {
                return BTNodeState::Failure;
            }

            // Пробуем детей по порядку
            while (currentChildIndex < children.size()) {
                BTNodeState childState = children[currentChildIndex]->Execute(gameObject, blackboard, deltaTime);

                switch (childState) {
                case BTNodeState::Success:
                    // Ребенок успешен - селектор успешен
                    return BTNodeState::Success;

                case BTNodeState::Failure:
                    // Ребенок провалился - пробуем следующего
                    currentChildIndex++;
                    break;

                case BTNodeState::Running:
                    // Ребенок еще выполняется - ждем
                    return BTNodeState::Running;

                case BTNodeState::Aborted:
                    // Ребенок прерван - прерываем селектор
                    return BTNodeState::Aborted;
                }
            }

            // Все дети провалились - селектор проваливается
            return BTNodeState::Failure;
        }

        // При выходе - сбрасываем индекс
        void OnExit(GameObject* gameObject, Blackboard* blackboard) override {
            currentChildIndex = 0;
        }
    };

}