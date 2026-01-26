#pragma once
#include "BTDecorator.h"

// ============================================================================
// BTRetryUntilSuccess.h
// Декоратор "Повтор до успеха" - повторяет пока не получит Success
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // BTRetryUntilSuccess - повторяет ребенка, пока не получит Success
    // 
    // ЛОГИКА:
    // - Выполняет ребенка
    // - Если Success - возвращает Success (останавливается)
    // - Если Failure - сбрасывает ребенка и пробует снова
    // - Если Running - ждет завершения
    // - Можно задать максимальное количество попыток
    // 
    // ПРИМЕР ИСПОЛЬЗОВАНИЯ:
    // RetryUntilSuccess(5) {  ← Максимум 5 попыток
    //     TryOpenDoor()       ← Попытаться открыть дверь
    // }
    // Будет пробовать открыть дверь, пока не получится
    // Если за 5 попыток не получилось - вернет Failure
    // ------------------------------------------------------------------------
    class BTRetryUntilSuccess : public BTDecorator {
    public:
        // --------------------------------------------------------------------
        // Конструктор
        // maxAttempts: максимум попыток (-1 = бесконечно)
        // --------------------------------------------------------------------
        BTRetryUntilSuccess(int maxAttempts = -1, const std::string& name = "RetryUntilSuccess")
            : BTDecorator(name), maxRetries(maxAttempts), currentAttempt(0) {
        }

    protected:
        // --------------------------------------------------------------------
        // При входе - сбрасываем счетчик попыток
        // --------------------------------------------------------------------
        void OnEnter(GameObject* gameObject, Blackboard* blackboard) override {
            currentAttempt = 0;
        }

        // --------------------------------------------------------------------
        // Выполняем ребенка с повторами при неудаче
        // --------------------------------------------------------------------
        BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) override {
            // Проверка наличия ребенка
            if (!child) {
                return BTNodeState::Failure;
            }

            // Бесконечные попытки
            if (maxRetries == -1) {
                BTNodeState childState = child->Execute(gameObject, blackboard, deltaTime);

                switch (childState) {
                case BTNodeState::Success:
                    // Успех - завершаем
                    return BTNodeState::Success;

                case BTNodeState::Running:
                    // Еще выполняется - ждем
                    return BTNodeState::Running;

                case BTNodeState::Failure:
                    // Неудача - сбрасываем и пробуем снова
                    child->Reset();
                    return BTNodeState::Running;

                case BTNodeState::Aborted:
                    // Прерван
                    return BTNodeState::Aborted;
                }
            }

            // Ограниченное количество попыток
            while (currentAttempt < maxRetries) {
                BTNodeState childState = child->Execute(gameObject, blackboard, deltaTime);

                switch (childState) {
                case BTNodeState::Success:
                    // Успех - завершаем
                    return BTNodeState::Success;

                case BTNodeState::Running:
                    // Еще выполняется - ждем
                    return BTNodeState::Running;

                case BTNodeState::Failure:
                    // Неудача - пробуем снова
                    currentAttempt++;
                    child->Reset();

                    // Если это была последняя попытка - возвращаем Failure
                    if (currentAttempt >= maxRetries) {
                        return BTNodeState::Failure;
                    }
                    break;

                case BTNodeState::Aborted:
                    // Прерван
                    return BTNodeState::Aborted;
                }
            }

            // Исчерпаны все попытки
            return BTNodeState::Failure;
        }

        // --------------------------------------------------------------------
        // При выходе - сбрасываем счетчик
        // --------------------------------------------------------------------
        void OnExit(GameObject* gameObject, Blackboard* blackboard) override {
            currentAttempt = 0;
        }

    private:
        int maxRetries;       // Максимум попыток (-1 = бесконечно)
        int currentAttempt;   // Текущая попытка
    };

} // namespace AI