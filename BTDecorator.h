#pragma once
#include "BTNode.h"

// ============================================================================
// BTDecorator.h
// Базовый класс для декораторов (узлов с одним ребенком)
// ============================================================================

namespace AI {

    class BTDecorator : public BTNode {
    public:
        BTDecorator(const std::string& name = "Decorator")
            : BTNode(name), child(nullptr) {
        }

        virtual ~BTDecorator() = default;

        // Установить дочерний узел
        void SetChild(BTNodePtr childNode) {
            child = childNode;
        }

        // Получить дочерний узел
        BTNodePtr GetChild() const {
            return child;
        }

        // Проверить, есть ли ребенок
        bool HasChild() const {
            return child != nullptr;
        }

        // Сброс состояния - сбрасываем и ребенка
        void Reset() override {
            BTNode::Reset();
            if (child) {
                child->Reset();
            }
        }

    protected:
        // При прерывании - прерываем ребенка
        void OnAbort(GameObject* gameObject, Blackboard* blackboard) override {
            if (child && child->IsRunning()) {
                child->Abort(gameObject, blackboard);
            }
        }

    protected:
        BTNodePtr child;  // Единственный дочерний узел
    };

}