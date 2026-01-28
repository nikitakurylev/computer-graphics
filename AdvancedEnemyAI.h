#pragma once
#include "AIComponent.h"
#include "BTSequence.h"
#include "BTSelector.h"
#include "BTAction.h"
#include "BTCondition.h"
#include "BTInverter.h"
#include "UtilitySelector.h"
#include "UtilityAction.h"
#include "UtilityConsideration.h"
#include "SimpleMath.h"
#include <vector>

using namespace DirectX::SimpleMath;

// ============================================================================
// AdvancedEnemyAI.h
// ============================================================================

namespace AI {

    // ------------------------------------------------------------------------
    // Типы поведения врагов
    // ------------------------------------------------------------------------
    enum class EnemyType {
        Curious,    // Любопытный - следит за игроком издалека
        Collector   // Собиратель - ищет монеты
    };

    class AdvancedEnemyAI : public AIComponent {
    public:
        AdvancedEnemyAI(EnemyType type = EnemyType::Curious)
            : AIComponent(), enemyType(type)
        {
            // Настройки по умолчанию
            moveSpeed = 3.0f;
            detectionRange = 20.0f;
            comfortDistance = 8.0f;
            personalSpaceRadius = 3.0f;
            collectionRadius = 2.0f; // Радиус сбора монет
            minHeight = 3.0f;        // Минимальная высота
            maxHeight = 10.0f;       // Максимальная высота

            InitializeBlackboard();
        }

        // Start - инициализация
        void Start() override {
            auto transform = gameObject->GetTransform();
            startPosition = transform->position;

            // Ограничиваем начальную высоту
            if (startPosition.y < minHeight) startPosition.y = minHeight;
            if (startPosition.y > maxHeight) startPosition.y = maxHeight;
            transform->position = startPosition;

            GetBlackboard()->SetValue("StartPosition", startPosition);
            GetBlackboard()->SetValue("EnemyType", static_cast<int>(enemyType));

            CreateBehaviourTreeForType();

            std::cout << "[AdvancedEnemy] Type: " << GetTypeName()
                << " started at (" << startPosition.x << ", "
                << startPosition.y << ", " << startPosition.z << ")" << std::endl;

            AIComponent::Start();
        }

        // Update - обновление
        void Update(float deltaTime) override {
            UpdateBlackboard(deltaTime);

            // Ограничиваем высоту до выполнения AI
            ClampHeight();

            AIComponent::Update(deltaTime);

            // Ограничиваем высоту после выполнения AI
            ClampHeight();

            // Проверяем сбор монет (если Collector)
            if (enemyType == EnemyType::Collector) {
                CheckCoinCollection();
            }
        }

        // Настройки
        void SetMoveSpeed(float speed) { moveSpeed = speed; }
        void SetDetectionRange(float range) { detectionRange = range; }
        void SetComfortDistance(float distance) { comfortDistance = distance; }
        void SetHeightRange(float minH, float maxH) {
            minHeight = minH;
            maxHeight = maxH;
        }

    private:
        // ИНИЦИАЛИЗАЦИЯ

        void InitializeBlackboard() {
            auto bb = GetBlackboard();
            bb->SetValue("MoveSpeed", moveSpeed);
            bb->SetValue("DetectionRange", detectionRange);
            bb->SetValue("ComfortDistance", comfortDistance);
            bb->SetValue("PersonalSpaceRadius", personalSpaceRadius);
            bb->SetValue("PlayerDetected", false);
            bb->SetValue("PlayerDistance", 999.0f);
            bb->SetValue("IsPlayerMoving", false);
            bb->SetValue("TimeSincePlayerStopped", 0.0f);
            bb->SetValue("IsObserving", false);
        }

        void UpdateBlackboard(float deltaTime) {
            auto bb = GetBlackboard();
            auto game = gameObject->GetGame();
            auto transform = gameObject->GetTransform();

            // Позиция врага
            Vector3 enemyPos = transform->position;
            bb->SetValue("EnemyPosition", enemyPos);

            // Данные об игроке
            Vector3 playerPos = game->cam_pos;
            float distance = Vector3::Distance(enemyPos, playerPos);

            bb->SetValue("PlayerPosition", playerPos);
            bb->SetValue("PlayerDistance", distance);
            bb->SetValue("PlayerDetected", distance <= detectionRange);

            Vector3 toPlayer = playerPos - enemyPos;
            toPlayer.Normalize();
            bb->SetValue("DirectionToPlayer", toPlayer);

            // Определяем, двигается ли игрок
            static Vector3 lastPlayerPos = playerPos;
            float playerMovement = Vector3::Distance(playerPos, lastPlayerPos);
            bool isPlayerMoving = playerMovement > 0.05f;

            bb->SetValue("IsPlayerMoving", isPlayerMoving);

            // Таймер остановки игрока
            float timeStopped = bb->GetValue<float>("TimeSincePlayerStopped", 0.0f);
            if (isPlayerMoving) {
                timeStopped = 0.0f;
            }
            else {
                timeStopped += deltaTime;
            }
            bb->SetValue("TimeSincePlayerStopped", timeStopped);

            lastPlayerPos = playerPos;

            // Поиск ближайшей монеты (для Collector)
            if (enemyType == EnemyType::Collector) {
                UpdateNearestCoinInfo(game);
            }
        }

        void UpdateNearestCoinInfo(Game* game) {
            auto bb = GetBlackboard();
            auto enemyPos = gameObject->GetTransform()->position;

            GameObject* nearestCoin = nullptr;
            float nearestDistance = 999999.0f;

            // Ищем ближайшую монету
            for (GameObject* obj : game->GameObjects) {
                if (obj == gameObject) continue;
                if (obj->GetScriptingComponents().empty()) continue;

                // Проверяем что объект не под землей (уже собран)
                if (obj->GetTransform()->position.y < 0) continue;

                float dist = Vector3::Distance(enemyPos, obj->GetTransform()->position);
                if (dist < nearestDistance) {
                    nearestDistance = dist;
                    nearestCoin = obj;
                }
            }

            if (nearestCoin) {
                bb->SetValue("NearestCoinPosition", nearestCoin->GetTransform()->position);
                bb->SetValue("NearestCoinDistance", nearestDistance);
                bb->SetValue("HasNearestCoin", true);
                bb->SetValue("NearestCoinPtr", reinterpret_cast<intptr_t>(nearestCoin));
            }
            else {
                bb->SetValue("HasNearestCoin", false);
            }
        }

        // СБОР МОНЕТ
        void CheckCoinCollection() {
            auto bb = GetBlackboard();
            if (!bb->GetValue<bool>("HasNearestCoin", false)) return;

            float distance = bb->GetValue<float>("NearestCoinDistance", 999.0f);

            // Если близко - собираем
            if (distance < collectionRadius) {
                intptr_t coinPtr = bb->GetValue<intptr_t>("NearestCoinPtr", 0);
                if (coinPtr != 0) {
                    GameObject* coin = reinterpret_cast<GameObject*>(coinPtr);

                    // "Собираем" монету - отправляем под землю
                    coin->GetTransform()->position = Vector3(0, -1000, 0);
                    coin->receive_transform_from_backend = false;
                    coin->send_transform_to_backend = true;

                    std::cout << "[Collector] ★ Collected coin!" << std::endl;

                    // Сбрасываем данные о монете
                    bb->SetValue("HasNearestCoin", false);
                }
            }
        }

        // ОГРАНИЧЕНИЕ ВЫСОТЫ
        void ClampHeight() {
            auto transform = gameObject->GetTransform();
            Vector3 pos = transform->position;

            if (pos.y < minHeight) {
                pos.y = minHeight;
                transform->position = pos;
            }
            else if (pos.y > maxHeight) {
                pos.y = maxHeight;
                transform->position = pos;
            }
        }

        // СОЗДАНИЕ BEHAVIOUR TREE
        void CreateBehaviourTreeForType() {
            switch (enemyType) {
            case EnemyType::Curious:
                SetBehaviourTree(CreateCuriousTree());
                break;
            case EnemyType::Collector:
                SetBehaviourTree(CreateCollectorTree());
                break;
            }
        }

        // CURIOUS ENEMY (Любопытный)
        BTNodePtr CreateCuriousTree() {
            auto root = std::make_shared<BTSelector>("CuriousRoot");

            // Ветка 1: Если игрок обнаружен → реагируем
            auto playerBranch = std::make_shared<BTSequence>("PlayerReaction");
            playerBranch->AddChild(CreatePlayerDetectedCondition());
            playerBranch->AddChild(CreateCuriousBehaviorSelector());
            root->AddChild(playerBranch);

            // Ветка 2: Иначе патрулируем
            root->AddChild(CreateIdleHoverAction());

            return root;
        }

        BTNodePtr CreateCuriousBehaviorSelector() {
            auto utilitySelector = std::make_shared<UtilitySelector>("CuriousDecision");

            // Действие 1: Приблизиться (если игрок стоит долго)
            auto approachAction = std::make_shared<LambdaUtilityAction>("Approach",
                [this](GameObject* go, Blackboard* bb, float dt) {
                    bb->SetValue("IsObserving", false);
                    MoveTowardsPlayer(go, bb, dt, 0.7f);
                    return BTNodeState::Success;
                }
            );

            approachAction->AddConsideration(std::make_shared<LambdaConsideration>("PlayerStoppedTime",
                [](GameObject* go, Blackboard* bb) {
                    float timeStopped = bb->GetValue<float>("TimeSincePlayerStopped", 0.0f);
                    return (std::min)(1.0f, timeStopped / 3.0f);
                }
            ));

            approachAction->AddConsideration(std::make_shared<LambdaConsideration>("NotTooClose",
                [](GameObject* go, Blackboard* bb) {
                    float dist = bb->GetValue<float>("PlayerDistance", 999.0f);
                    float comfort = bb->GetValue<float>("ComfortDistance", 8.0f);
                    return (dist > comfort) ? 0.8f : 0.0f;
                }
            ));

            // Действие 2: Наблюдать (СТОЯТЬ НА МЕСТЕ)
            auto observeAction = std::make_shared<LambdaUtilityAction>("Observe",
                [this](GameObject* go, Blackboard* bb, float dt) {
                    bb->SetValue("IsObserving", true);
                    return BTNodeState::Success;
                }
            );

            observeAction->AddConsideration(std::make_shared<LambdaConsideration>("PlayerMoving",
                [](GameObject* go, Blackboard* bb) {
                    bool moving = bb->GetValue<bool>("IsPlayerMoving", false);
                    float dist = bb->GetValue<float>("PlayerDistance", 999.0f);
                    float comfort = bb->GetValue<float>("ComfortDistance", 8.0f);

                    // Высокая полезность если игрок двигается на расстоянии
                    if (moving && dist >= comfort - 2.0f && dist <= comfort + 2.0f) {
                        return 0.95f;
                    }
                    return 0.1f;
                }
            ));

            // Действие 3: Отступить (если слишком близко)
            auto retreatAction = std::make_shared<LambdaUtilityAction>("Retreat",
                [this](GameObject* go, Blackboard* bb, float dt) {
                    bb->SetValue("IsObserving", false);
                    MoveAwayFromPlayer(go, bb, dt);
                    return BTNodeState::Success;
                }
            );

            retreatAction->AddConsideration(std::make_shared<LambdaConsideration>("TooClose",
                [](GameObject* go, Blackboard* bb) {
                    float dist = bb->GetValue<float>("PlayerDistance", 999.0f);
                    float personal = bb->GetValue<float>("PersonalSpaceRadius", 3.0f);
                    float comfort = bb->GetValue<float>("ComfortDistance", 8.0f);

                    // Критическая важность если очень близко
                    if (dist < personal) return 1.0f;
                    // Высокая важность если ближе комфортной зоны
                    if (dist < comfort - 1.0f) return 0.7f;
                    return 0.0f;
                }
            ));

            utilitySelector->AddChild(approachAction);
            utilitySelector->AddChild(observeAction);
            utilitySelector->AddChild(retreatAction);

            return utilitySelector;
        }

        // COLLECTOR ENEMY (Собиратель)
        BTNodePtr CreateCollectorTree() {
            auto root = std::make_shared<BTSelector>("CollectorRoot");

            // Ветка 1: Если есть монета → летим к ней
            auto coinBranch = std::make_shared<BTSequence>("CoinCollection");
            coinBranch->AddChild(CreateHasCoinCondition());
            coinBranch->AddChild(CreateCollectCoinAction());
            root->AddChild(coinBranch);

            // Ветка 2: Иначе патрулируем
            root->AddChild(CreateIdleHoverAction());

            return root;
        }

        BTNodePtr CreateCollectCoinAction() {
            return std::make_shared<BTLambdaAction>("CollectCoin",
                [this](GameObject* go, Blackboard* bb, float dt) {
                    Vector3 enemyPos = go->GetTransform()->position;
                    Vector3 coinPos = bb->GetValue<Vector3>("NearestCoinPosition");
                    float speed = bb->GetValue<float>("MoveSpeed", 3.0f);
                    float distance = bb->GetValue<float>("NearestCoinDistance", 999.0f);

                    // Если близко - замедляемся
                    if (distance < 5.0f) {
                        speed *= 0.5f;
                    }

                    Vector3 direction = coinPos - enemyPos;
                    direction.Normalize();

                    // Двигаемся к монете
                    Vector3 newPos = enemyPos + direction * speed * dt;

                    // Сохраняем высоту монеты
                    newPos.y = coinPos.y;

                    go->GetTransform()->position = newPos;

                    return BTNodeState::Running;
                }
            );
        }


        // УСЛОВИЯ (Conditions)
        BTNodePtr CreatePlayerDetectedCondition() {
            return std::make_shared<BTLambdaCondition>("IsPlayerDetected",
                [](GameObject* go, Blackboard* bb) {
                    bool detected = bb->GetValue<bool>("PlayerDetected", false);
                    return detected ? BTNodeState::Success : BTNodeState::Failure;
                }
            );
        }

        BTNodePtr CreateHasCoinCondition() {
            return std::make_shared<BTLambdaCondition>("HasNearestCoin",
                [](GameObject* go, Blackboard* bb) {
                    bool hasCoin = bb->GetValue<bool>("HasNearestCoin", false);
                    return hasCoin ? BTNodeState::Success : BTNodeState::Failure;
                }
            );
        }


        // ДЕЙСТВИЯ (Actions)
        BTNodePtr CreateIdleHoverAction() {
            return std::make_shared<BTLambdaAction>("IdleHover",
                [this](GameObject* go, Blackboard* bb, float dt) {
                    static float hoverTime = 0.0f;
                    static Vector3 hoverTarget = go->GetTransform()->position;

                    hoverTime += dt;

                    if (hoverTime > 3.0f) {
                        Vector3 startPos = bb->GetValue<Vector3>("StartPosition");
                        float angle = (rand() % 360) * (DirectX::XM_PI / 180.0f);
                        float dist = 3.0f + (rand() % 5);

                        hoverTarget = startPos + Vector3(
                            cos(angle) * dist,
                            0.0f,
                            sin(angle) * dist
                        );
                        hoverTarget.y = startPos.y;
                        hoverTime = 0.0f;
                    }

                    Vector3 enemyPos = go->GetTransform()->position;
                    Vector3 direction = hoverTarget - enemyPos;
                    direction.Normalize();

                    float speed = bb->GetValue<float>("MoveSpeed", 3.0f) * 0.5f;
                    go->GetTransform()->position = enemyPos + direction * speed * dt;

                    return BTNodeState::Running;
                }
            );
        }

        // ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДВИЖЕНИЯ
        void MoveTowardsPlayer(GameObject* go, Blackboard* bb, float dt, float speedMultiplier = 1.0f) {
            Vector3 enemyPos = go->GetTransform()->position;
            Vector3 playerPos = bb->GetValue<Vector3>("PlayerPosition");
            float speed = bb->GetValue<float>("MoveSpeed", 3.0f) * speedMultiplier;

            Vector3 direction = playerPos - enemyPos;

            // Сохраняем высоту врага (не летим вверх/вниз за игроком)
            direction.y = 0;
            direction.Normalize();

            Vector3 newPos = enemyPos + direction * speed * dt;
            newPos.y = enemyPos.y; // Сохраняем высоту

            go->GetTransform()->position = newPos;
        }

        void MoveAwayFromPlayer(GameObject* go, Blackboard* bb, float dt) {
            Vector3 enemyPos = go->GetTransform()->position;
            Vector3 playerPos = bb->GetValue<Vector3>("PlayerPosition");
            float speed = bb->GetValue<float>("MoveSpeed", 3.0f);

            Vector3 direction = enemyPos - playerPos;

            // Сохраняем высоту
            direction.y = 0;
            direction.Normalize();

            Vector3 newPos = enemyPos + direction * speed * dt;
            newPos.y = enemyPos.y;

            go->GetTransform()->position = newPos;
        }

        const char* GetTypeName() const {
            switch (enemyType) {
            case EnemyType::Curious: return "Curious";
            case EnemyType::Collector: return "Collector";
            default: return "Unknown";
            }
        }

    private:
        EnemyType enemyType;
        Vector3 startPosition;

        // Параметры
        float moveSpeed;
        float detectionRange;
        float comfortDistance;
        float personalSpaceRadius;
        float collectionRadius;
        float minHeight;
        float maxHeight;
    };

} // namespace AI