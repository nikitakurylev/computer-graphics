#include "SkeletalAnimatorComponent.h"
#include <algorithm>
#include <cmath>

// ============================================================
SkeletalAnimatorComponent::SkeletalAnimatorComponent(Skeleton* skeleton)
    : skeleton_(skeleton)
{
}

// ============================================================
void SkeletalAnimatorComponent::SetClip(const SkeletalAnimationClip* clip)
{
    clip_ = clip;
    playbackTime_ = 0.0;
}

// ============================================================
void SkeletalAnimatorComponent::Update(float deltaTime)
{
    if (!clip_ || !playing || !skeleton_) return;

    // --- Продвигаем время ---
    playbackTime_ += deltaTime * speed;

    if (loop)
        playbackTime_ = std::fmod(playbackTime_, clip_->duration);
    else
        playbackTime_ = (std::min)(playbackTime_, clip_->duration);

    // --- Обновляем localTransform каждой кости ---
    for (auto& bone : skeleton_->bones)
    {
        auto it = clip_->channels.find(bone.name);
        if (it == clip_->channels.end())
        {
            // Для этой кости анимации нет — восстанавливаем T-pose
            bone.localTransform = bone.bindPoseLocalTransform;
            continue;
        }

        const BoneAnimChannel& ch = it->second;

        // Сэмплируем каналы — если кривая пустая, берём из bindPose
        Vector3    pos;
        Quaternion rot;
        Vector3    scale;

        if (!ch.posKeys.empty())
            pos = SampleVec(ch.posKeys, playbackTime_);
        else
        {
            // Извлекаем позицию из bindPose
            bone.bindPoseLocalTransform.Decompose(scale, rot, pos);
        }

        if (!ch.rotKeys.empty())
            rot = SampleQuat(ch.rotKeys, playbackTime_);
        else
        {
            Vector3 dummyS, dummyT;
            bone.bindPoseLocalTransform.Decompose(dummyS, rot, dummyT);
        }

        if (!ch.scaleKeys.empty())
            scale = SampleVec(ch.scaleKeys, playbackTime_);
        else
        {
            Quaternion dummyR; Vector3 dummyT;
            bone.bindPoseLocalTransform.Decompose(scale, dummyR, dummyT);
        }

        // TRS: сначала Scale, потом Rotation, потом Translation
        // DirectXMath SimpleMath: Matrix::CreateScale * CreateFromQuaternion * CreateTranslation
        bone.localTransform =
            Matrix::CreateScale(scale) *
            Matrix::CreateFromQuaternion(rot) *
            Matrix::CreateTranslation(pos);
    }
}

// ============================================================
Vector3 SkeletalAnimatorComponent::SampleVec(
    const std::vector<AnimKey<Vector3>>& keys, double t)
{
    if (keys.size() == 1) return keys[0].value;
    if (t <= keys.front().time) return keys.front().value;
    if (t >= keys.back().time)  return keys.back().value;

    // Бинарный поиск нужного интервала
    int lo = 0, hi = (int)keys.size() - 1;
    while (hi - lo > 1)
    {
        int mid = (lo + hi) / 2;
        if (keys[mid].time <= t) lo = mid;
        else                     hi = mid;
    }

    double range = keys[hi].time - keys[lo].time;
    if (range < 1e-9) return keys[lo].value;

    float alpha = (float)((t - keys[lo].time) / range);
    return Vector3::Lerp(keys[lo].value, keys[hi].value, alpha);
}

// ============================================================
Quaternion SkeletalAnimatorComponent::SampleQuat(
    const std::vector<AnimKey<Quaternion>>& keys, double t)
{
    if (keys.size() == 1) return keys[0].value;
    if (t <= keys.front().time) return keys.front().value;
    if (t >= keys.back().time)  return keys.back().value;

    int lo = 0, hi = (int)keys.size() - 1;
    while (hi - lo > 1)
    {
        int mid = (lo + hi) / 2;
        if (keys[mid].time <= t) lo = mid;
        else                     hi = mid;
    }

    double range = keys[hi].time - keys[lo].time;
    if (range < 1e-9) return keys[lo].value;

    float alpha = (float)((t - keys[lo].time) / range);
    // Slerp для корректной интерполяции вращений
    return Quaternion::Slerp(keys[lo].value, keys[hi].value, alpha);
}