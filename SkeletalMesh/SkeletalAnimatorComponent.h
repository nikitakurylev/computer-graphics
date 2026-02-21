#pragma once

#include "../Component.h"
#include "SkeletalMesh.h"
#include "SkeletalAnimation.h"

// ============================================================
//  SkeletalAnimatorComponent
//
//  Обновляет localTransform каждой кости в Skeleton каждый кадр
//  на основе текущего времени и загруженного клипа.
//  SkeletalModelComponent затем читает обновлённый скелет
//  в своём Draw() и отправляет bone palette в шейдер.
//
//  Добавляется на тот же GameObject что и SkeletalModelComponent.
// ============================================================
class SkeletalAnimatorComponent : public Component
{
public:
    explicit SkeletalAnimatorComponent(Skeleton* skeleton);

    // Устанавливает клип для воспроизведения
    void SetClip(const SkeletalAnimationClip* clip);

    void Update(float deltaTime) override;

    bool  loop    = true;
    bool  playing = true;
    float speed   = 1.0f;

    double GetPlaybackTime() const { return playbackTime_; }
    void   SetPlaybackTime(double t) { playbackTime_ = t; }

private:
    Skeleton*                    skeleton_;
    const SkeletalAnimationClip* clip_ = nullptr;
    double                       playbackTime_ = 0.0;

    // Сэмплирует значение кривой в момент времени t (с линейной интерполяцией)
    template<typename T>
    static T Sample(const std::vector<AnimKey<T>>& keys, double t);

    static Vector3    SampleVec(const std::vector<AnimKey<Vector3>>&    keys, double t);
    static Quaternion SampleQuat(const std::vector<AnimKey<Quaternion>>& keys, double t);
};
