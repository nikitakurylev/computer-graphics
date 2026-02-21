#pragma once

#define NOMINMAX
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "SkeletalAnimation.h"

// ============================================================
//  Загружает все анимационные клипы из FBX/GLB/etc. файла.
//  Не требует Device/Context — чисто CPU данные.
// ============================================================
class SkeletalAnimationLoader
{
public:
    // Загружает файл и возвращает список клипов.
    // Возвращает пустой вектор если файл не найден или анимаций нет.
    static std::vector<SkeletalAnimationClip> Load(const std::string& filename);

private:
    static SkeletalAnimationClip ProcessAnimation(const aiAnimation* anim);

    static std::vector<AnimKey<Vector3>> ExtractPositionKeys(const aiNodeAnim* ch, double ticksPerSec);
    static std::vector<AnimKey<Quaternion>> ExtractRotationKeys(const aiNodeAnim* ch, double ticksPerSec);
    static std::vector<AnimKey<Vector3>> ExtractScaleKeys(const aiNodeAnim* ch, double ticksPerSec);
};
