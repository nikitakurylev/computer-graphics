#pragma once
// SkeletalDebug.h — временный файл для диагностики
// Добавьте #include "SkeletalDebug.h" в SkeletalModelLoader.cpp
// и вызовите DumpSkeleton() после загрузки скелета

#include <fstream>
#include <string>
#include "SkeletalMesh.h"
#include "SkeletalAnimation.h"

inline void DumpSkeleton(const Skeleton& sk, const std::string& path = "skeleton_dump.txt")
{
    std::ofstream f(path);
    f << "=== SKELETON DUMP === bones: " << sk.bones.size() << "\n\n";
    for (int i = 0; i < (int)sk.bones.size(); i++)
    {
        const Bone& b = sk.bones[i];
        // Decompose требует неконстантный объект — копируем матрицы
        Matrix lt = b.localTransform;
        Matrix bpt = b.bindPoseLocalTransform;

        Vector3 pos, scale; Quaternion rot;
        lt.Decompose(scale, rot, pos);

        Vector3 bpos, bscale; Quaternion brot;
        bpt.Decompose(bscale, brot, bpos);

        f << "[" << i << "] " << b.name << "  parent=" << b.parentIndex << "\n";
        f << "  localT  pos=(" << pos.x << "," << pos.y << "," << pos.z << ")"
            << "  rot=(" << rot.x << "," << rot.y << "," << rot.z << "," << rot.w << ")"
            << "  scale=(" << scale.x << "," << scale.y << "," << scale.z << ")\n";
        f << "  bindT   pos=(" << bpos.x << "," << bpos.y << "," << bpos.z << ")"
            << "  rot=(" << brot.x << "," << brot.y << "," << brot.z << "," << brot.w << ")\n";
        f << "  hasPreRot=" << b.hasPreRotation << "\n";
        if (b.hasPreRotation)
        {
            Matrix pm = b.preRotation;
            Vector3 pp, ps; Quaternion pq;
            pm.Decompose(ps, pq, pp);
            f << "  preRot quat=(" << pq.x << "," << pq.y << "," << pq.z << "," << pq.w << ")\n";
        }
        // offsetMatrix diagonal
        f << "  offsetMatrix[0][0]=" << b.offsetMatrix._11
            << "  [3][0]=" << b.offsetMatrix._41
            << "  [3][1]=" << b.offsetMatrix._42
            << "  [3][2]=" << b.offsetMatrix._43 << "\n";
        f << "\n";
    }
}

inline void DumpAnimClip(const SkeletalAnimationClip& clip, const std::string& path = "anim_dump.txt")
{
    std::ofstream f(path);
    f << "=== ANIM CLIP: " << clip.name << " ===\n";
    f << "duration=" << clip.duration << "s  ticksPerSec=" << clip.ticksPerSec << "\n";
    f << "channels: " << clip.channels.size() << "\n\n";

    for (auto& [name, ch] : clip.channels)
    {
        f << "[" << name << "]\n";
        f << "  posKeys=" << ch.posKeys.size()
            << "  rotKeys=" << ch.rotKeys.size()
            << "  scaleKeys=" << ch.scaleKeys.size() << "\n";

        if (!ch.posKeys.empty())
        {
            auto& k0 = ch.posKeys[0];
            f << "  pos[0] t=" << k0.time
                << " xyz=(" << k0.value.x << "," << k0.value.y << "," << k0.value.z << ")\n";
        }
        if (!ch.rotKeys.empty())
        {
            auto& k0 = ch.rotKeys[0];
            f << "  rot[0] t=" << k0.time
                << " xyzw=(" << k0.value.x << "," << k0.value.y
                << "," << k0.value.z << "," << k0.value.w << ")\n";
        }
    }
}