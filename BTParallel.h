#pragma once
#include "BTCompositeNode.h"

// ============================================================================
// BTParallel.h
// Узел "Параллель" - выполняет всех детей одновременно
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // Политика успеха для параллельного узла
    // ------------------------------------------------------------------------
    enum class ParallelPolicy {
        RequireOne,  // Успех, если хотя бы один ребенок успешен
        RequireAll   // Успех, только если все дети успешны
    };

    // ------------------------------------------------------------------------
    // BTParallel - выполняет все дочерние узлы параллельно (в одном кадре)
    // 
    // ЛОГИКА:
    // - Выполняет ВСЕХ детей каждый кадр
    // - Успех/Неудача зависит от политики (RequireOne/RequireAll)
    // - Возвращает Running, пока хотя бы один ребенок Running
    // 
    // ПОЛИТИКИ:
    // RequireOne - успех если >= 1 ребенок Success
    // RequireAll - успех только если ВСЕ дети Success
    // 
    // ПРИМЕР ИСПОЛЬЗОВАНИЯ:
    // Parallel(RequireAll) {
    //     PlayAnimation()    ← Проигрывать анимацию
    //     PlaySound()        ← Одновременно играть звук
    //     MoveForward()      ← И двигаться вперед
    // }
    // Все действия выполняются одновременно
    // ------------------------------------------------------------------------
    class BTParallel : public BTCompositeNode {
    public:
        BTParallel(ParallelPolicy policy = ParallelPolicy::RequireAll,
            const std::string& name = "Parallel")
            : BTCompositeNode(name), successPolicy(policy) {
        }

    protected:
        // --------------------------------------------------------------------
        // Выполнение параллельных задач
        // --------------------------------------------------------------------
        BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) override {
            // Проверка на отсутствие детей
            if (children.empty()) {
                return BTNodeState::Success;
            }

            int successCount = 0;
            int failureCount = 0;
            int runningCount = 0;

            // Выполняем ВСЕХ детей
            for (size_t i = 0; i < children.size(); i++) {
                BTNodeState childState = children[i]->Execute(gameObject, blackboard, deltaTime);

                switch (childState) {
                case BTNodeState::Success:
                    successCount++;
                    break;

                case BTNodeState::Failure:
                    failureCount++;
                    break;

                case BTNodeState::Running:
                    runningCount++;
                    break;

                case BTNodeState::Aborted:
                    // Если хоть один прерван - прерываем всех
                    return BTNodeState::Aborted;
                }
            }

            // Если кто-то еще выполняется - ждем
            if (runningCount > 0) {
                return BTNodeState::Running;
            }

            // Проверяем политику успеха
            if (successPolicy == ParallelPolicy::RequireAll) {
                // Все должны быть успешны
                return (successCount == children.size()) ? BTNodeState::Success : BTNodeState::Failure;
            }
            else {
                // Хотя бы один должен быть успешен
                return (successCount > 0) ? BTNodeState::Success : BTNodeState::Failure;
            }
        }

        // --------------------------------------------------------------------
        // При прерывании - прерываем всех детей
        // --------------------------------------------------------------------
        void OnAbort(GameObject* gameObject, Blackboard* blackboard) override {
            for (auto& child : children) {
                if (child->IsRunning()) {
                    child->Abort(gameObject, blackboard);
                }
            }
        }

    private:
        ParallelPolicy successPolicy;  // Политика определения успеха
    };

} // namespace AI