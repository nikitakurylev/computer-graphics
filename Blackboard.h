#pragma once
#include <map>
#include <string>
#include <memory>
#include <iostream>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

// ============================================================================
// Blackboard.h
// Система "черной доски" для обмена данными между узлами Behaviour Tree
// ============================================================================

namespace AI {

    // BlackboardValue - базовый класс для хранения значений
    // Использует полиморфизм для хранения любых типов
    class BlackboardValue {
    public:
        virtual ~BlackboardValue() = default;
        virtual BlackboardValue* Clone() const = 0;
    };

    // BlackboardValueImpl - шаблонный класс для хранения конкретного типа
    template<typename T>
    class BlackboardValueImpl : public BlackboardValue {
    public:
        explicit BlackboardValueImpl(const T& val) : value(val) {}

        T value;

        BlackboardValue* Clone() const override {
            return new BlackboardValueImpl<T>(value);
        }
    };


    class Blackboard {
    public:
        Blackboard() = default;

        ~Blackboard() {
            Clear();
        }

        // Конструктор копирования
        Blackboard(const Blackboard& other) {
            for (const auto& pair : other.data) {
                data[pair.first] = std::shared_ptr<BlackboardValue>(pair.second->Clone());
            }
        }

        // Оператор присваивания
        Blackboard& operator=(const Blackboard& other) {
            if (this != &other) {
                Clear();
                for (const auto& pair : other.data) {
                    data[pair.first] = std::shared_ptr<BlackboardValue>(pair.second->Clone());
                }
            }
            return *this;
        }


        // Установить значение в Blackboard
        template<typename T>
        void SetValue(const std::string& key, const T& value) {
            data[key] = std::make_shared<BlackboardValueImpl<T>>(value);

#ifdef AI_DEBUG
            std::cout << "[Blackboard] Set: " << key << std::endl;
#endif
        }


        // Получить значение из Blackboard
        template<typename T>
        T GetValue(const std::string& key, const T& defaultValue = T()) const {
            auto it = data.find(key);
            if (it != data.end()) {
                // Пытаемся привести к нужному типу
                auto* typedValue = dynamic_cast<BlackboardValueImpl<T>*>(it->second.get());
                if (typedValue) {
                    return typedValue->value;
                }
                else {
#ifdef AI_DEBUG
                    std::cout << "[Blackboard] ERROR: Type mismatch for key '" << key << "'" << std::endl;
#endif
                    return defaultValue;
                }
            }
            return defaultValue;
        }


        // Получить указатель на значение (для модификации)
        template<typename T>
        T* GetValuePtr(const std::string& key) {
            auto it = data.find(key);
            if (it != data.end()) {
                auto* typedValue = dynamic_cast<BlackboardValueImpl<T>*>(it->second.get());
                if (typedValue) {
                    return &(typedValue->value);
                }
            }
            return nullptr;
        }


        // Проверить, существует ли ключ в Blackboard
        bool HasValue(const std::string& key) const {
            return data.find(key) != data.end();
        }


        // Удалить значение из Blackboard
        void RemoveValue(const std::string& key) {
            data.erase(key);

#ifdef AI_DEBUG
            std::cout << "[Blackboard] Removed: " << key << std::endl;
#endif
        }


        // Очистить весь Blackboard
        void Clear() {
            data.clear();

#ifdef AI_DEBUG
            std::cout << "[Blackboard] Cleared all data" << std::endl;
#endif
        }


        // Вывести все данные в консоль (для отладки)
        void DebugPrint() const {
            std::cout << "\n=== Blackboard Contents ===" << std::endl;
            std::cout << "Total keys: " << data.size() << std::endl;
            for (const auto& pair : data) {
                std::cout << "  - " << pair.first << std::endl;
            }
            std::cout << "==========================\n" << std::endl;
        }


        // Получить количество записей
        size_t GetSize() const {
            return data.size();
        }

    private:
        // Хранилище данных: ключ (string) -> значение (полиморфный указатель)
        std::map<std::string, std::shared_ptr<BlackboardValue>> data;
    };

}