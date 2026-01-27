#pragma once
#include "BTCompositeNode.h"

// ============================================================================
// BTSequence.h
// Узел "Последовательность" - выполняет детей по порядку
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // BTSequence - выполняет дочерние узлы последовательно
    // 
    // ЛОГИКА:
    // - Выполняет детей слева направо
    // - Если ребенок вернул Success - переходит к следующему
    // - Если ребенок вернул Failure - возвращает Failure (останавливается)
    // - Если ребенок вернул Running - возвращает Running (продолжит на след. кадре)
    // - Если ВСЕ дети вернули Success - возвращает Success
    // 
    // АНАЛОГ: логическое И (AND)
    // 
    // ПРИМЕР ИСПОЛЬЗОВАНИЯ:
    // Sequence {
    //     CheckAmmo()      ← Есть ли патроны?
    //     AimAtTarget()    ← Прицелиться
    //     Shoot()          ← Выстрелить
    // }
    // Если нет патронов - вся последовательность провалится
    // ------------------------------------------------------------------------
    class BTSequence : public BTCompositeNode {
    public:
        BTSequence(const std::string& name = "Sequence")
            : BTCompositeNode(name) {
        }

    protected:
        // --------------------------------------------------------------------
        // При входе - начинаем с первого ребенка
        // --------------------------------------------------------------------
        void OnEnter(GameObject* gameObject, Blackboard* blackboard) override {
            currentChildIndex = 0;
        }

        // --------------------------------------------------------------------
        // Выполнение последовательности
        // --------------------------------------------------------------------
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

        // --------------------------------------------------------------------
        // При выходе - сбрасываем индекс
        // --------------------------------------------------------------------
        void OnExit(GameObject* gameObject, Blackboard* blackboard) override {
            currentChildIndex = 0;
        }
    };

} // namespace AI