#pragma once
#include <functional>
#include <memory>
#include <cmath>
#include "Blackboard.h"

// ============================================================================
// UtilityConsideration.h
// Система расчёта отдельных факторов для Utility AI
// ============================================================================

class GameObject;

namespace AI {

    // ------------------------------------------------------------------------
    // ResponseCurve - тип кривой отклика для нормализации значений
    // 
    // Определяет, как входное значение преобразуется в полезность (0-1)
    // ------------------------------------------------------------------------
    enum class ResponseCurve {
        Linear,         // y = x (линейная зависимость)
        Quadratic,      // y = x^2 (возрастает быстрее)
        InverseQuadratic, // y = 1 - (1-x)^2 (возрастает медленнее)
        Sigmoid,        // S-образная кривая (плавный переход)
        InverseSigmoid, // Обратная S-образная
        Constant        // Всегда возвращает 1.0
    };

    // ------------------------------------------------------------------------
    // UtilityConsideration - отдельный фактор для расчёта полезности
    // 
    // Consideration отвечает за оценку ОДНОГО параметра:
    // - Расстояние до цели
    // - Уровень здоровья
    // - Количество патронов
    // - Видимость врага
    // 
    // Возвращает значение от 0.0 (минимальная полезность) до 1.0 (максимальная)
    // 
    // ПРИМЕР:
    // Consideration "Health" может возвращать:
    // - 1.0 если здоровье 100%
    // - 0.5 если здоровье 50%
    // - 0.0 если здоровье 0%
    // ------------------------------------------------------------------------
    class UtilityConsideration {
    public:
        UtilityConsideration(const std::string& name = "Consideration")
            : considerationName(name), curve(ResponseCurve::Linear), weight(1.0f) {
        }

        virtual ~UtilityConsideration() = default;

        // --------------------------------------------------------------------
        // Рассчитать полезность (0.0 - 1.0)
        // 
        // Параметры:
        // - gameObject: объект, для которого считаем
        // - blackboard: данные для расчёта
        // --------------------------------------------------------------------
        float Evaluate(GameObject* gameObject, Blackboard* blackboard) {
            // Получаем сырое значение (0-1)
            float rawValue = CalculateRawValue(gameObject, blackboard);

            // Ограничиваем 0-1
            rawValue = (std::max)(0.0f, (std::min)(1.0f, rawValue));

            // Применяем кривую отклика
            float curveValue = ApplyResponseCurve(rawValue);

            // Применяем вес
            return curveValue * weight;
        }

        // --------------------------------------------------------------------
        // Установить кривую отклика
        // --------------------------------------------------------------------
        void SetResponseCurve(ResponseCurve curveType) {
            curve = curveType;
        }

        // --------------------------------------------------------------------
        // Установить вес (важность этого фактора)
        // Обычно от 0.0 до 1.0, но может быть больше для "критических" факторов
        // --------------------------------------------------------------------
        void SetWeight(float w) {
            weight = w;
        }

        // --------------------------------------------------------------------
        // Получить имя
        // --------------------------------------------------------------------
        const std::string& GetName() const {
            return considerationName;
        }

    protected:
        // --------------------------------------------------------------------
        // Рассчитать сырое значение (0.0 - 1.0)
        // ПЕРЕОПРЕДЕЛИТЬ в наследниках!
        // --------------------------------------------------------------------
        virtual float CalculateRawValue(GameObject* gameObject, Blackboard* blackboard) = 0;

    private:
        // --------------------------------------------------------------------
        // Применить кривую отклика для нормализации
        // --------------------------------------------------------------------
        float ApplyResponseCurve(float input) {
            switch (curve) {
            case ResponseCurve::Linear:
                return input;

            case ResponseCurve::Quadratic:
                return input * input;

            case ResponseCurve::InverseQuadratic:
                return 1.0f - (1.0f - input) * (1.0f - input);

            case ResponseCurve::Sigmoid:
                // S-образная кривая: 1 / (1 + e^(-10*(x-0.5)))
                return 1.0f / (1.0f + std::exp(-10.0f * (input - 0.5f)));

            case ResponseCurve::InverseSigmoid:
                // Обратная S-образная
                return 1.0f - (1.0f / (1.0f + std::exp(-10.0f * (input - 0.5f))));

            case ResponseCurve::Constant:
                return 1.0f;

            default:
                return input;
            }
        }

    private:
        std::string considerationName;
        ResponseCurve curve;
        float weight;
    };

    // ------------------------------------------------------------------------
    // Умный указатель на Consideration
    // ------------------------------------------------------------------------
    using ConsiderationPtr = std::shared_ptr<UtilityConsideration>;

    // ------------------------------------------------------------------------
    // LambdaConsideration - Consideration через лямбда-функцию
    // 
    // Для быстрого создания без отдельных классов
    // 
    // ПРИМЕР:
    // auto healthConsideration = std::make_shared<LambdaConsideration>("Health",
    //     [](GameObject* go, Blackboard* bb) {
    //         float health = bb->GetValue<float>("Health", 100.0f);
    //         return health / 100.0f; // Нормализуем 0-1
    //     }
    // );
    // ------------------------------------------------------------------------
    class LambdaConsideration : public UtilityConsideration {
    public:
        using EvalFunc = std::function<float(GameObject*, Blackboard*)>;

        LambdaConsideration(const std::string& name, EvalFunc func)
            : UtilityConsideration(name), evalFunc(func) {
        }

    protected:
        float CalculateRawValue(GameObject* gameObject, Blackboard* blackboard) override {
            if (evalFunc) {
                return evalFunc(gameObject, blackboard);
            }
            return 0.0f;
        }

    private:
        EvalFunc evalFunc;
    };

} // namespace AI