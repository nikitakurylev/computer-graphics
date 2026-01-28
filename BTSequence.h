#pragma once
#include "BTCompositeNode.h"

// ============================================================================
// BTSequence.h
// Узел  выполняет детей по порядку
// ============================================================================

namespace AI {

    class BTSequence : public BTCompositeNode {
    public:
        BTSequence(const std::string& name = "Sequence")
            : BTCompositeNode(name) {
        }

    protected:

        // При входе - начинаем с первого ребенка
        void OnEnter(GameObject* gameObject, Blackboard* blackboard) override {
            currentChildIndex = 0;
        }

        // Выполнение последовательности
        BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) override {
            // Проверка на отсутствие детей
            if (children.empty()) {
                return BTNodeState::Success;
            }

            // Выполняем детей по порядку
            while (currentChildIndex < children.size()) {
                BTNodeState childState = children[currentChildIndex]->Execute(gameObject, blackboard, deltaTime);

                switch (childState) {
                case BTNodeState::Success:
                    // Ребенок успешен - переходим к следующему
                    currentChildIndex++;
                    break;

                case BTNodeState::Failure:
                    // Ребенок провалился - вся последовательность проваливается
                    return BTNodeState::Failure;

                case BTNodeState::Running:
                    // Ребенок еще выполняется - ждем
                    return BTNodeState::Running;

                case BTNodeState::Aborted:
                    // Ребенок прерван - прерываем всю последовательность
                    return BTNodeState::Aborted;
                }
            }

            // Все дети успешны - последовательность успешна
            return BTNodeState::Success;
        }

        // При выходе - сбрасываем индекс
        void OnExit(GameObject* gameObject, Blackboard* blackboard) override {
            currentChildIndex = 0;
        }
    };

}