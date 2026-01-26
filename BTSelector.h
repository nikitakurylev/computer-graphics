#pragma once
#include "BTCompositeNode.h"

// ============================================================================
// BTSelector.h
// Узел "Селектор" - выбирает первого успешного ребенка
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // BTSelector - пробует дочерние узлы по порядку до первого успеха
    // 
    // ЛОГИКА:
    // - Выполняет детей слева направо
    // - Если ребенок вернул Success - возвращает Success (останавливается)
    // - Если ребенок вернул Failure - переходит к следующему
    // - Если ребенок вернул Running - возвращает Running (продолжит на след. кадре)
    // - Если ВСЕ дети вернули Failure - возвращает Failure
    // 
    // АНАЛОГ: логическое ИЛИ (OR)
    // 
    // ПРИМЕР ИСПОЛЬЗОВАНИЯ:
    // Selector {
    //     AttackEnemy()     ← Попробовать атаковать (если враг рядом)
    //     ChaseEnemy()      ← Если нет - преследовать (если враг виден)
    //     Patrol()          ← Если нет - патрулировать
    // }
    // Выберет первое доступное действие
    // ------------------------------------------------------------------------
    class BTSelector : public BTCompositeNode {
    public:
        BTSelector(const std::string& name = "Selector")
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
        // Выполнение селектора
        // --------------------------------------------------------------------
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

        // --------------------------------------------------------------------
        // При выходе - сбрасываем индекс
        // --------------------------------------------------------------------
        void OnExit(GameObject* gameObject, Blackboard* blackboard) override {
            currentChildIndex = 0;
        }
    };

} // namespace AI