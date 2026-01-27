#pragma once
#include "BTDecorator.h"

// ============================================================================
// BTRepeat.h
// Декоратор "Повтор" - повторяет выполнение ребенка
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // BTRepeat - повторяет выполнение дочернего узла N раз
    // 
    // ЛОГИКА:
    // - Выполняет ребенка, пока не достигнет лимита повторов
    // - Если repeatCount = -1, повторяет бесконечно
    // - Сбрасывает ребенка после каждого выполнения
    // - Возвращает Running, пока не достигнут лимит
    // 
    // ПРИМЕР ИСПОЛЬЗОВАНИЯ:
    // Repeat(3) {           ← Повторить 3 раза
    //     Patrol()          ← Патрулировать
    // }
    // 
    // Repeat(-1) {          ← Повторять бесконечно
    //     WaitAndListen()
    // }
    // ------------------------------------------------------------------------
    class BTRepeat : public BTDecorator {
    public:
        // --------------------------------------------------------------------
        // Конструктор
        // repeatCount: количество повторов (-1 = бесконечно)
        // --------------------------------------------------------------------
        BTRepeat(int repeatCount = -1, const std::string& name = "Repeat")
            : BTDecorator(name), maxRepeats(repeatCount), currentRepeat(0) {
        }

    protected:
        // --------------------------------------------------------------------
        // При входе - сбрасываем счетчик
        // --------------------------------------------------------------------
        void OnEnter(GameObject* gameObject, Blackboard* blackboard) override {
            currentRepeat = 0;
        }

        // --------------------------------------------------------------------
        // Выполняем ребенка с повторами
        // --------------------------------------------------------------------
        BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) override {
            // Проверка наличия ребенка
            if (!child) {
                return BTNodeState::Failure;
            }

            // Бесконечное повторение
            if (maxRepeats == -1) {
                BTNodeState childState = child->Execute(gameObject, blackboard, deltaTime);

                // Если ребенок завершился - сбрасываем его и продолжаем
                if (childState != BTNodeState::Running) {
                    child->Reset();
                }

                // Всегда возвращаем Running для бесконечного цикла
                return BTNodeState::Running;
            }

            // Ограниченное повторение
            while (currentRepeat < maxRepeats) {
                BTNodeState childState = child->Execute(gameObject, blackboard, deltaTime);

                switch (childState) {
                case BTNodeState::Running:
                    // Ребенок еще выполняется - ждем
                    return BTNodeState::Running;

                case BTNodeState::Aborted:
                    // Прерван - прерываем повтор
                    return BTNodeState::Aborted;

                case BTNodeState::Success:
                case BTNodeState::Failure:
                    // Ребенок завершился - сбрасываем и переходим к след. повтору
                    currentRepeat++;
                    child->Reset();
                    break;
                }
            }

            // Все повторы выполнены
            return BTNodeState::Success;
        }

        // --------------------------------------------------------------------
        // При выходе - сбрасываем счетчик
        // --------------------------------------------------------------------
        void OnExit(GameObject* gameObject, Blackboard* blackboard) override {
            currentRepeat = 0;
        }

    private:
        int maxRepeats;      // Максимальное количество повторов (-1 = бесконечно)
        int currentRepeat;   // Текущий номер повтора
    };

} // namespace AI