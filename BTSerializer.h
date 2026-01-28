#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include "BTNode.h"
#include "BTCompositeNode.h"
#include "BTSequence.h"
#include "BTSelector.h"
#include "BTParallel.h"
#include "BTDecorator.h"
#include "BTInverter.h"
#include "BTRepeat.h"
#include "BTRetryUntilSuccess.h"
#include "BTAction.h"
#include "BTCondition.h"

// ============================================================================
// BTSerializer.h
// Сериализует дерево поведения используя JSON
// ============================================================================

namespace AI {

    class BTSerializer {
    public:

        using ActionFactory = std::function<BTNodePtr(const std::string& type, const std::string& name)>;

        // Сохраням дерево в JSON файл
        static bool SaveToFile(BTNodePtr root, const std::string& filepath) {
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }

            std::string json = SerializeNode(root, 0);
            file << json;
            file.close();
            return true;
        }

        // Загружаем дерево из JSON файла
        static BTNodePtr LoadFromFile(const std::string& filepath, ActionFactory actionFactory) {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                return nullptr;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string json = buffer.str();
            file.close();

            size_t pos = 0;
            return DeserializeNode(json, pos, actionFactory);
        }

    private:

        // Сериализация узла в JSON строку
        static std::string SerializeNode(BTNodePtr node, int indent) {
            if (!node) {
                return Indent(indent) + "null";
            }

            std::ostringstream oss;
            oss << Indent(indent) << "{\n";

            // Имя узла
            oss << Indent(indent + 1) << "\"name\": \"" << EscapeString(node->GetName()) << "\",\n";

            // Тип узла
            std::string type = GetNodeType(node);
            oss << Indent(indent + 1) << "\"type\": \"" << type << "\"";

            if (auto composite = std::dynamic_pointer_cast<BTCompositeNode>(node)) {
                oss << ",\n" << Indent(indent + 1) << "\"children\": [\n";

                size_t childCount = composite->GetChildCount();
                for (size_t i = 0; i < childCount; i++) {
                    // Получаем ребенка по индексу
                    if (i > 0) oss << ",\n";
                }

                oss << "\n" << Indent(indent + 1) << "]";
            }
            else if (auto decorator = std::dynamic_pointer_cast<BTDecorator>(node)) {
                if (auto child = decorator->GetChild()) {
                    oss << ",\n" << Indent(indent + 1) << "\"child\": \n";
                    oss << SerializeNode(child, indent + 1);
                }

                if (auto repeat = std::dynamic_pointer_cast<BTRepeat>(node)) {
                }
            }

            oss << "\n" << Indent(indent) << "}";
            return oss.str();
        }

        // Десериализация узла из JSON строки
        static BTNodePtr DeserializeNode(const std::string& json, size_t& pos, ActionFactory actionFactory) {
            SkipWhitespace(json, pos);

            if (json.substr(pos, 4) == "null") {
                pos += 4;
                return nullptr;
            }

            if (json[pos] != '{') {
                return nullptr;
            }
            pos++; // Skip '{'

            std::string name;
            std::string type;
            std::vector<BTNodePtr> children;
            BTNodePtr child = nullptr;

            // Parse object
            while (pos < json.length()) {
                SkipWhitespace(json, pos);

                if (json[pos] == '}') {
                    pos++; 
                    break;
                }

                if (json[pos] == ',') {
                    pos++;
                    continue;
                }

                std::string key = ParseString(json, pos);
                SkipWhitespace(json, pos);

                if (json[pos] != ':') {
                    return nullptr;
                }
                pos++;

                SkipWhitespace(json, pos);

                // Парсим значение в зависимости от типа
                if (key == "name") {
                    name = ParseString(json, pos);
                }
                else if (key == "type") {
                    type = ParseString(json, pos);
                }
                else if (key == "children") {
                    children = ParseArray(json, pos, actionFactory);
                }
                else if (key == "child") {
                    child = DeserializeNode(json, pos, actionFactory);
                }
            }

            // Создаем узел в зависимости от типа
            return CreateNode(type, name, children, child, actionFactory);
        }

        // Вспомогательные функции
        static std::string GetNodeType(BTNodePtr node) {
            if (std::dynamic_pointer_cast<BTSequence>(node)) return "Sequence";
            if (std::dynamic_pointer_cast<BTSelector>(node)) return "Selector";
            if (std::dynamic_pointer_cast<BTParallel>(node)) return "Parallel";
            if (std::dynamic_pointer_cast<BTInverter>(node)) return "Inverter";
            if (std::dynamic_pointer_cast<BTRepeat>(node)) return "Repeat";
            if (std::dynamic_pointer_cast<BTRetryUntilSuccess>(node)) return "RetryUntilSuccess";
            if (std::dynamic_pointer_cast<BTCondition>(node)) return "Condition";
            if (std::dynamic_pointer_cast<BTAction>(node)) return "Action";
            return "Unknown";
        }

        static BTNodePtr CreateNode(const std::string& type, const std::string& name,
            const std::vector<BTNodePtr>& children,
            BTNodePtr child,
            ActionFactory actionFactory) {
            BTNodePtr node = nullptr;

            // Композитные узла
            if (type == "Sequence") {
                node = std::make_shared<BTSequence>(name);
            }
            else if (type == "Selector") {
                node = std::make_shared<BTSelector>(name);
            }
            else if (type == "Parallel") {
                node = std::make_shared<BTParallel>(ParallelPolicy::RequireAll, name);
            }
            // Декораторы
            else if (type == "Inverter") {
                node = std::make_shared<BTInverter>(name);
            }
            else if (type == "Repeat") {
                node = std::make_shared<BTRepeat>(-1, name);
            }
            else if (type == "RetryUntilSuccess") {
                node = std::make_shared<BTRetryUntilSuccess>(-1, name);
            }
            else if (type == "Action" || type == "Condition") {
                if (actionFactory) {
                    node = actionFactory(type, name);
                }
            }

            if (!node) {
                return nullptr;
            }

            if (auto composite = std::dynamic_pointer_cast<BTCompositeNode>(node)) {
                for (auto& childNode : children) {
                    composite->AddChild(childNode);
                }
            }

            if (auto decorator = std::dynamic_pointer_cast<BTDecorator>(node)) {
                if (child) {
                    decorator->SetChild(child);
                }
            }

            return node;
        }

        static std::string Indent(int level) {
            return std::string(level * 2, ' ');
        }

        static std::string EscapeString(const std::string& str) {
            std::string result;
            for (char c : str) {
                if (c == '"') result += "\\\"";
                else if (c == '\\') result += "\\\\";
                else if (c == '\n') result += "\\n";
                else if (c == '\t') result += "\\t";
                else result += c;
            }
            return result;
        }

        static void SkipWhitespace(const std::string& json, size_t& pos) {
            while (pos < json.length() && std::isspace(json[pos])) {
                pos++;
            }
        }

        static std::string ParseString(const std::string& json, size_t& pos) {
            if (json[pos] != '"') {
                return "";
            }
            pos++;

            std::string result;
            while (pos < json.length() && json[pos] != '"') {
                if (json[pos] == '\\' && pos + 1 < json.length()) {
                    pos++;
                    if (json[pos] == 'n') result += '\n';
                    else if (json[pos] == 't') result += '\t';
                    else result += json[pos];
                }
                else {
                    result += json[pos];
                }
                pos++;
            }
            pos++;
            return result;
        }

        static std::vector<BTNodePtr> ParseArray(const std::string& json, size_t& pos, ActionFactory actionFactory) {
            std::vector<BTNodePtr> result;

            if (json[pos] != '[') {
                return result;
            }
            pos++;

            while (pos < json.length()) {
                SkipWhitespace(json, pos);

                if (json[pos] == ']') {
                    pos++;
                    break;
                }

                if (json[pos] == ',') {
                    pos++;
                    continue;
                }

                auto node = DeserializeNode(json, pos, actionFactory);
                if (node) {
                    result.push_back(node);
                }
            }

            return result;
        }
    };

}