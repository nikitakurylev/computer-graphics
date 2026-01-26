#pragma once
#include "Component.h"
#include "BTNode.h"
#include "Blackboard.h"
#include <memory>

// ============================================================================
// AIComponent.h
// Компонент для подключения AI к GameObject
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // AIComponent - компонент искусственного интеллекта для GameObject
    // 
    // ИНТЕГРАЦИЯ В ДВИЖОК!
    // 
    // Использование:
    // 1. Создать дерево поведения (Behaviour Tree)
    // 2. Создать AIComponent
    // 3. Установить дерево в компонент
    // 4. Добавить компонент к GameObject
    // 5. AI автоматически выполняется в Update()
    // 
    // ПРИМЕР:
    // auto aiComponent = new AIComponent();
    // aiComponent->SetBehaviourTree(myTree);
    // enemyGameObject->AddComponent(aiComponent);
    // ------------------------------------------------------------------------
    class AIComponent : public Component {
    public:
        AIComponent()
            : behaviourTree(nullptr),
            blackboard(std::make_shared<Blackboard>()),
            enabled(true),
            debugMode(false) {
        }

        virtual ~AIComponent() = default;

        // --------------------------------------------------------------------
        // Установить дерево поведения
        // --------------------------------------------------------------------
        void SetBehaviourTree(BTNodePtr tree) {
            behaviourTree = tree;
        }

        // --------------------------------------------------------------------
        // Получить дерево поведения
        // --------------------------------------------------------------------
        BTNodePtr GetBehaviourTree() const {
            return behaviourTree;
        }

        // --------------------------------------------------------------------
        // Получить Blackboard (для записи/чтения данных извне)
        // --------------------------------------------------------------------
        std::shared_ptr<Blackboard> GetBlackboard() const {
            return blackboard;
        }

        // --------------------------------------------------------------------
        // Включить/выключить AI
        // --------------------------------------------------------------------
        void SetEnabled(bool enable) {
            enabled = enable;
        }

        bool IsEnabled() const {
            return enabled;
        }

        // --------------------------------------------------------------------
        // Включить/выключить режим отладки
        // В режиме отладки выводится информация о выполнении узлов
        // --------------------------------------------------------------------
        void SetDebugMode(bool debug) {
            debugMode = debug;
        }

        bool IsDebugMode() const {
            return debugMode;
        }

        // --------------------------------------------------------------------
        // Сбросить AI (перезапустить дерево)
        // --------------------------------------------------------------------
        void Reset() {
            if (behaviourTree) {
                behaviourTree->Reset();
            }
        }

        // --------------------------------------------------------------------
        // Очистить Blackboard
        // --------------------------------------------------------------------
        void ClearBlackboard() {
            if (blackboard) {
                blackboard->Clear();
            }
        }

        // --------------------------------------------------------------------
        // Start - вызывается при инициализации
        // --------------------------------------------------------------------
        void Start() override {
            // Инициализация AI (можно добавить начальные данные в blackboard)
            if (debugMode) {
                std::cout << "[AIComponent] Started on GameObject" << std::endl;
            }
        }

        // --------------------------------------------------------------------
        // Update - выполняется каждый кадр
        // Здесь запускается дерево поведения
        // --------------------------------------------------------------------
        void Update(float deltaTime) override {
            // Если AI выключен или нет дерева - ничего не делаем
            if (!enabled || !behaviourTree) {
                return;
            }

            // Выполняем дерево поведения
            BTNodeState state = behaviourTree->Execute(gameObject, blackboard.get(), deltaTime);

            // Отладочный вывод
            if (debugMode) {
                std::cout << "[AIComponent] Behaviour Tree state: "
                    << BTNodeStateToString(state) << std::endl;
            }

            // Если дерево завершилось (не Running), можно его сбросить
            // для повторного выполнения в следующем кадре
            if (state != BTNodeState::Running) {
                // Опционально: можно автоматически сбрасывать
                // behaviourTree->Reset();
            }
        }

    private:
        BTNodePtr behaviourTree;                    // Дерево поведения
        std::shared_ptr<Blackboard> blackboard;     // Данные для AI
        bool enabled;                               // Включен ли AI
        bool debugMode;                             // Режим отладки
    };

} // namespace AI