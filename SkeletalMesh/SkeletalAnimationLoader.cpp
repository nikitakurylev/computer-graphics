#include "SkeletalAnimationLoader.h"
#include <Windows.h>

// ============================================================
std::vector<SkeletalAnimationClip> SkeletalAnimationLoader::Load(const std::string& filename)
{
    // Устанавливаем рабочую директорию туда, где лежит exe
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring wbuf(buffer);
    auto pos = wbuf.find_last_of(L"\\/");
    SetCurrentDirectory(wbuf.substr(0, pos).c_str());

    Assimp::Importer importer;
    // Тот же флаг что и в SkeletalModelLoader — без $AssimpFbx$ узлов,
    // каналы анимации напрямую соответствуют именам костей в скелете.
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_ConvertToLeftHanded |
        aiProcess_PopulateArmatureData);

    if (!scene)
    {
        MessageBoxA(nullptr, importer.GetErrorString(), "SkeletalAnimationLoader: failed to load", MB_ICONWARNING);
        return {};
    }

    if (!scene->HasAnimations())
    {
        MessageBoxA(nullptr, ("No animations found in: " + filename).c_str(),
            "SkeletalAnimationLoader", MB_ICONWARNING);
        return {};
    }

    std::vector<SkeletalAnimationClip> clips;
    clips.reserve(scene->mNumAnimations);

    for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
        clips.push_back(ProcessAnimation(scene->mAnimations[i]));

    return clips;
}

// ============================================================
SkeletalAnimationClip SkeletalAnimationLoader::ProcessAnimation(const aiAnimation* anim)
{
    SkeletalAnimationClip clip;
    clip.name = anim->mName.C_Str();
    clip.ticksPerSec = (anim->mTicksPerSecond > 0.0) ? anim->mTicksPerSecond : 30.0;
    clip.duration = anim->mDuration / clip.ticksPerSec; // переводим тики → секунды

    for (UINT i = 0; i < anim->mNumChannels; ++i)
    {
        const aiNodeAnim* ch = anim->mChannels[i];
        std::string boneName = ch->mNodeName.C_Str();

        BoneAnimChannel channel;
        channel.name = boneName;
        channel.posKeys = ExtractPositionKeys(ch, clip.ticksPerSec);
        channel.rotKeys = ExtractRotationKeys(ch, clip.ticksPerSec);
        channel.scaleKeys = ExtractScaleKeys(ch, clip.ticksPerSec);

        clip.channels[boneName] = std::move(channel);
    }

    return clip;
}

// ============================================================
std::vector<AnimKey<Vector3>> SkeletalAnimationLoader::ExtractPositionKeys(
    const aiNodeAnim* ch, double ticksPerSec)
{
    std::vector<AnimKey<Vector3>> keys;
    keys.reserve(ch->mNumPositionKeys);
    for (UINT i = 0; i < ch->mNumPositionKeys; ++i)
    {
        const auto& k = ch->mPositionKeys[i];
        keys.push_back({ k.mTime / ticksPerSec,
            Vector3((float)k.mValue.x, (float)k.mValue.y, (float)k.mValue.z) });
    }
    return keys;
}

// ============================================================
std::vector<AnimKey<Quaternion>> SkeletalAnimationLoader::ExtractRotationKeys(
    const aiNodeAnim* ch, double ticksPerSec)
{
    std::vector<AnimKey<Quaternion>> keys;
    keys.reserve(ch->mNumRotationKeys);
    for (UINT i = 0; i < ch->mNumRotationKeys; ++i)
    {
        const auto& k = ch->mRotationKeys[i];
        // Assimp Quaternion: w,x,y,z → DirectXMath: x,y,z,w
        keys.push_back({ k.mTime / ticksPerSec,
            Quaternion((float)k.mValue.x, (float)k.mValue.y,
                       (float)k.mValue.z, (float)k.mValue.w) });
    }
    return keys;
}

// ============================================================
std::vector<AnimKey<Vector3>> SkeletalAnimationLoader::ExtractScaleKeys(
    const aiNodeAnim* ch, double ticksPerSec)
{
    std::vector<AnimKey<Vector3>> keys;
    keys.reserve(ch->mNumScalingKeys);
    for (UINT i = 0; i < ch->mNumScalingKeys; ++i)
    {
        const auto& k = ch->mScalingKeys[i];
        keys.push_back({ k.mTime / ticksPerSec,
            Vector3((float)k.mValue.x, (float)k.mValue.y, (float)k.mValue.z) });
    }
    return keys;
}