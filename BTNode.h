#pragma once
#include <string>
#include <vector>
#include <memory>
#include "BTNodeState.h"
#include "Blackboard.h"

// ============================================================================
// BTNode.h
// Базовый класс для всех узлов Behaviour Tree
// ============================================================================

class GameObject; // Forward declaration

namespace AI {

    // ------------------------------------------------------------------------
    // BTNode - абстрактный базовый класс для всех узлов дерева поведения
    // 
    // Все узлы (Sequence, Selector, Action и т.д.) наследуются от этого класса
    // 
    // Жизненный цикл узла:
    // 1. OnEnter() - вызывается при первом входе в узел
    // 2. Tick() - вызывается каждый кадр, пока узел активен
    // 3. OnExit() - вызывается при выходе из узла
    // ------------------------------------------------------------------------
    class BTNode {
    public:
        // --------------------------------------------------------------------
        // Конструктор
        // name - имя узла для отладки
        // --------------------------------------------------------------------
        BTNode(const std::string& name = "BTNode")
            : nodeName(name), isRunning(false) {
        }

        virtual ~BTNode() = default;

        // --------------------------------------------------------------------
        // Главный метод выполнения узла
        // 
        // Вызывается каждый кадр для обновления узла
        // Возвращает состояние выполнения (Success/Failure/Running/Aborted)
        // 
        // Параметры:
        // - gameObject: GameObject, к которому прикреплен AI
        // - blackboard: общее хранилище данных
        // - deltaTime: время с предыдущего кадра
        // --------------------------------------------------------------------
        BTNodeState Execute(GameObject* gameObject, Blackboard* blackboard, float deltaTime) {
            // Если узел только начинает выполняться - вызываем OnEnter
            if (!isRunning) {
                OnEnter(gameObject, blackboard);
                isRunning = true;
            }

            // Выполняем основную логику узла
            BTNodeState state = Tick(gameObject, blackboard, deltaTime);

            // Если узел завершился (не Running) - вызываем OnExit
            if (state != BTNodeState::Running) {
                OnExit(gameObject, blackboard);
                isRunning = false;
            }

            return state;
        }

        // --------------------------------------------------------------------
        // Принудительное прерывание узла
        // Переводит узел в состояние Aborted
        // --------------------------------------------------------------------
        void Abort(GameObject* gameObject, Blackboard* blackboard) {
            if (isRunning) {
                OnAbort(gameObject, blackboard);
                OnExit(gameObject, blackboard);
                isRunning = false;
            }
        }

        // --------------------------------------------------------------------
        // Сброс состояния узла
        // Используется для перезапуска узла
        // --------------------------------------------------------------------
        virtual void Reset() {
            isRunning = false;
        }

        // --------------------------------------------------------------------
        // Получить имя узла (для отладки)
        // --------------------------------------------------------------------
        const std::string& GetName() const { return nodeName; }

        // --------------------------------------------------------------------
        // Проверить, выполняется ли узел в данный момент
        // --------------------------------------------------------------------
        bool IsRunning() const { return isRunning; }

    protected:
        // --------------------------------------------------------------------
        // ВИРТУАЛЬНЫЕ МЕТОДЫ - переопределяются в наследниках
        // --------------------------------------------------------------------

        // Вызывается при входе в узел (первый раз)
        // Здесь инициализируется состояние узла
        virtual void OnEnter(GameObject* gameObject, Blackboard* blackboard) {}

        // Основная логика узла - вызывается каждый кадр
        // ОБЯЗАТЕЛЬНО переопределить в наследниках!
        virtual BTNodeState Tick(GameObject* gameObject, Blackboard* blackboard, float deltaTime) = 0;

        // Вызывается при выходе из узла
        // Здесь происходит очистка ресурсов
        virtual void OnExit(GameObject* gameObject, Blackboard* blackboard) {}

        // Вызывается при принудительном прерывании
        virtual void OnAbort(GameObject* gameObject, Blackboard* blackboard) {}

    protected:
        std::string nodeName;  // Имя узла для отладки
        bool isRunning;        // Флаг активности узла
    };

    // ------------------------------------------------------------------------
    // Умный указатель на узел (используется для управления памятью)
    // ------------------------------------------------------------------------
    using BTNodePtr = std::shared_ptr<BTNode>;

} // namespace AI