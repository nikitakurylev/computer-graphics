#pragma once
#include "Component.h"
#include "SimpleMath.h"
#include <cstdlib>
#include <ctime>

using namespace DirectX::SimpleMath;

// ============================================================================
// SimpleFloatingEnemyAI.h
// Простой AI для летающего врага - случайное движение
// ============================================================================

// ----------------------------------------------------------------------------
// SimpleFloatingEnemyAI - враг, который медленно летает в случайных направлениях
// 
// ПОВЕДЕНИЕ:
// - Выбирает случайную точку в радиусе вокруг себя
// - Летит к этой точке
// - Когда достигает - выбирает новую случайную точку
// - Не реагирует на игрока (чисто декоративный)
// ----------------------------------------------------------------------------
class SimpleFloatingEnemyAI : public Component {
public:
    SimpleFloatingEnemyAI()
        : moveSpeed(2.0f),
        changeDirectionDistance(0.5f),
        maxDistance(10.0f),
        minHeight(3.0f),
        maxHeight(8.0f),
        timeSinceLastMove(0.0f),
        minTimeBetweenMoves(2.0f),
        maxTimeBetweenMoves(5.0f),
        nextMoveTime(0.0f)
    {
        // Инициализируем random seed (делается один раз)
        static bool randomInitialized = false;
        if (!randomInitialized) {
            srand(static_cast<unsigned int>(time(nullptr)));
            randomInitialized = true;
        }
    }

    // ------------------------------------------------------------------------
    // Start - инициализация
    // ------------------------------------------------------------------------
    void Start() override {
        auto transform = gameObject->GetTransform();
        startPosition = transform->position;

        // Выбираем первую случайную точку
        ChooseNewTargetPosition();

        std::cout << "[SimpleFloatingEnemy] Started at position: ("
            << startPosition.x << ", " << startPosition.y << ", " << startPosition.z << ")"
            << std::endl;
    }

    // ------------------------------------------------------------------------
    // Update - обновление каждый кадр
    // ------------------------------------------------------------------------
    void Update(float deltaTime) override {
        auto transform = gameObject->GetTransform();
        Vector3 currentPos = transform->position;

        timeSinceLastMove += deltaTime;

        // Проверяем, пора ли двигаться
        if (timeSinceLastMove < nextMoveTime) {
            // Ждём
            return;
        }

        // Вычисляем расстояние до целевой точки
        Vector3 toTarget = targetPosition - currentPos;
        float distanceToTarget = toTarget.Length();

        // Если достигли цели - выбираем новую
        if (distanceToTarget < changeDirectionDistance) {
            ChooseNewTargetPosition();
            timeSinceLastMove = 0.0f;
            return;
        }

        // Двигаемся к целевой точке
        toTarget.Normalize();
        Vector3 newPos = currentPos + toTarget * moveSpeed * deltaTime;

        // Ограничиваем движение по высоте
        if (newPos.y < minHeight) {
            newPos.y = minHeight;
        }
        else if (newPos.y > maxHeight) {
            newPos.y = maxHeight;
        }

        transform->position = newPos;
    }

    // ------------------------------------------------------------------------
    // Настройки (можно вызывать после создания)
    // ------------------------------------------------------------------------

    void SetMoveSpeed(float speed) {
        moveSpeed = speed;
    }

    void SetMaxDistance(float distance) {
        maxDistance = distance;
    }

    void SetHeightRange(float minH, float maxH) {
        minHeight = minH;
        maxHeight = maxH;
    }

    void SetTimeBetweenMoves(float minTime, float maxTime) {
        minTimeBetweenMoves = minTime;
        maxTimeBetweenMoves = maxTime;
    }

private:
    // ------------------------------------------------------------------------
    // Выбрать новую случайную целевую точку
    // ------------------------------------------------------------------------
    void ChooseNewTargetPosition() {
        // Случайный угол (0-360 градусов)
        float angle = GetRandomFloat(0.0f, DirectX::XM_2PI);

        // Случайное расстояние (0 - maxDistance)
        float distance = GetRandomFloat(0.0f, maxDistance);

        // Случайная высота
        float height = GetRandomFloat(minHeight, maxHeight);

        // Вычисляем новую точку относительно стартовой позиции
        targetPosition.x = startPosition.x + cos(angle) * distance;
        targetPosition.y = height;
        targetPosition.z = startPosition.z + sin(angle) * distance;

        // Случайное время до следующего движения
        nextMoveTime = GetRandomFloat(minTimeBetweenMoves, maxTimeBetweenMoves);

        std::cout << "[SimpleFloatingEnemy] New target: ("
            << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z
            << ") in " << nextMoveTime << "s" << std::endl;
    }

    // ------------------------------------------------------------------------
    // Получить случайное float значение в диапазоне [min, max]
    // ------------------------------------------------------------------------
    float GetRandomFloat(float min, float max) {
        float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return min + random * (max - min);
    }

private:
    // Параметры движения
    float moveSpeed;                    // Скорость движения
    float changeDirectionDistance;      // На каком расстоянии считаем, что достигли цели
    float maxDistance;                  // Максимальное расстояние от начальной точки
    float minHeight;                    // Минимальная высота полёта
    float maxHeight;                    // Максимальная высота полёта

    // Параметры паузы
    float minTimeBetweenMoves;          // Минимальное время паузы между движениями
    float maxTimeBetweenMoves;          // Максимальное время паузы между движениями
    float timeSinceLastMove;            // Время с последнего начала движения
    float nextMoveTime;                 // Когда начинать следующее движение

    // Позиции
    Vector3 startPosition;              // Начальная позиция (центр блуждания)
    Vector3 targetPosition;             // Текущая целевая точка
}; 