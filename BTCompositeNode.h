#pragma once
#include "BTNode.h"
#include <vector>

// ============================================================================
// BTCompositeNode.h
// Базовый класс для композитных узлов (узлов с дочерними элементами)
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // BTCompositeNode - узел, который имеет дочерние узлы
    // 
    // Композитные узлы выполняют своих детей в определенном порядке
    // и возвращают результат на основе их состояний
    // 
    // Наследники: Sequence, Selector, Parallel
    // ------------------------------------------------------------------------
    class BTCompositeNode : public BTNode {
    public:
        BTCompositeNode(const std::string& name = "CompositeNode")
            : BTNode(name), currentChildIndex(0) {
        }

        virtual ~BTCompositeNode() = default;

        // --------------------------------------------------------------------
        // Добавить дочерний узел
        // 
        // Пример:
        // auto sequence = std::make_shared<BTSequence>("MainSequence");
        // sequence->AddChild(std::make_shared<CheckHealthAction>());
        // sequence->AddChild(std::make_shared<AttackAction>());
        // --------------------------------------------------------------------
        void AddChild(BTNodePtr child) {
            children.push_back(child);
        }

        // --------------------------------------------------------------------
        // Получить количество дочерних узлов
        // --------------------------------------------------------------------
        size_t GetChildCount() const {
            return children.size();
        }

        // --------------------------------------------------------------------
        // Получить дочерний узел по индексу
        // --------------------------------------------------------------------
        BTNodePtr GetChild(size_t index) const {
            if (index < children.size()) {
                return children[index];
            }
            return nullptr;
        }

        // --------------------------------------------------------------------
        // Очистить всех детей
        // --------------------------------------------------------------------
        void ClearChildren() {
            children.clear();
            currentChildIndex = 0;
        }

        // --------------------------------------------------------------------
        // Сброс состояния - сбрасывает и всех детей
        // --------------------------------------------------------------------
        void Reset() override {
            BTNode::Reset();
            currentChildIndex = 0;
            for (auto& child : children) {
                child->Reset();
            }
        }

    protected:
        // --------------------------------------------------------------------
        // При прерывании - прерываем текущего дочернего узла
        // --------------------------------------------------------------------
        void OnAbort(GameObject* gameObject, Blackboard* blackboard) override {
            if (currentChildIndex < children.size()) {
                children[currentChildIndex]->Abort(gameObject, blackboard);
            }
        }

    protected:
        std::vector<BTNodePtr> children;    // Список дочерних узлов
        size_t currentChildIndex;           // Индекс текущего выполняемого ребенка
    };

} // namespace AI