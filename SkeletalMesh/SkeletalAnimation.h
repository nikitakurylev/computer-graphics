#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

// ============================================================
//  Ключ анимационной кривой
// ============================================================
template<typename T>
struct AnimKey
{
    double time;  // в секундах
    T      value;
};

// ============================================================
//  Канал одной кости: три кривые (pos, rot, scale)
// ============================================================
struct BoneAnimChannel
{
    std::string name; // имя кости (совпадает с Bone::name в Skeleton)

    std::vector<AnimKey<Vector3>>    posKeys;
    std::vector<AnimKey<Quaternion>> rotKeys;
    std::vector<AnimKey<Vector3>>    scaleKeys;
};

// ============================================================
//  Один анимационный клип
// ============================================================
struct SkeletalAnimationClip
{
    std::string name;
    double      duration;    // секунды
    double      ticksPerSec; // FPS тиков

    // Канал для каждой кости (индексируется по имени кости)
    std::unordered_map<std::string, BoneAnimChannel> channels;

    bool IsValid() const { return duration > 0.0 && !channels.empty(); }
};
