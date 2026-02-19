#pragma once
#include "Skeleton.h"
#include <vector>
#include <string>

// ============================================================================
// AnimationClip.h
// Keyframe animation data and playback
// ============================================================================

namespace Animation {

    // ------------------------------------------------------------------------
    // BoneKeyframe - single keyframe for a bone
    // ------------------------------------------------------------------------
    struct BoneKeyframe {
        float time;              // Time in seconds
        Vector3 position;        // Position at this keyframe
        Quaternion rotation;     // Rotation at this keyframe
        Vector3 scale;           // Scale at this keyframe
        
        BoneKeyframe() : time(0), scale(1, 1, 1) {}
        
        BoneKeyframe(float t, Vector3 pos, Quaternion rot, Vector3 scl)
            : time(t), position(pos), rotation(rot), scale(scl) {}
    };
    
    // ------------------------------------------------------------------------
    // BoneAnimationTrack - all keyframes for a single bone
    // ------------------------------------------------------------------------
    struct BoneAnimationTrack {
        int boneId;                                 // Which bone this track affects
        std::vector<BoneKeyframe> positionKeys;     // Position keyframes
        std::vector<BoneKeyframe> rotationKeys;     // Rotation keyframes
        std::vector<BoneKeyframe> scaleKeys;        // Scale keyframes
        
        BoneAnimationTrack() : boneId(-1) {}
    };
    
    // ------------------------------------------------------------------------
    // AnimationClip - complete animation data
    // 
    // Contains all animation tracks for all bones
    // Can be sampled at any time to update skeleton
    // ------------------------------------------------------------------------
    class AnimationClip {
    public:
        std::string name;                           // Animation name
        float duration;                             // Total duration in seconds
        float ticksPerSecond;                       // Playback speed
        std::vector<BoneAnimationTrack> tracks;     // Per-bone animation data
        
        AnimationClip() : duration(0), ticksPerSecond(25.0f) {}
        
        AnimationClip(const std::string& animName, float dur, float tps = 25.0f)
            : name(animName), duration(dur), ticksPerSecond(tps) {}
        
        // --------------------------------------------------------------------
        // Sample animation at given time and apply to skeleton
        // 
        // Parameters:
        //   time - time in seconds (will be wrapped to [0, duration])
        //   skeleton - skeleton to apply animation to
        // --------------------------------------------------------------------
        void Sample(float time, Skeleton* skeleton) {
            if (!skeleton || tracks.empty()) return;
            
            // Wrap time to animation duration
            time = fmod(time, duration);
            if (time < 0) time += duration;
            
            // Apply each track
            for (auto& track : tracks) {
                Bone* bone = skeleton->GetBone(track.boneId);
                if (!bone) continue;
                
                // Interpolate position
                Vector3 position = InterpolatePosition(track, time);
                
                // Interpolate rotation (use slerp for smooth rotation)
                Quaternion rotation = InterpolateRotation(track, time);
                
                // Interpolate scale
                Vector3 scale = InterpolateScale(track, time);
                
                // Build local transform matrix
                bone->localTransform = 
                    Matrix::CreateScale(scale) *
                    Matrix::CreateFromQuaternion(rotation) *
                    Matrix::CreateTranslation(position);
            }
        }
        
    private:
        // --------------------------------------------------------------------
        // Interpolate position at given time
        // --------------------------------------------------------------------
        Vector3 InterpolatePosition(const BoneAnimationTrack& track, float time) {
            if (track.positionKeys.empty()) return Vector3::Zero;
            if (track.positionKeys.size() == 1) return track.positionKeys[0].position;
            
            // Find keyframes before and after current time
            for (size_t i = 0; i < track.positionKeys.size() - 1; i++) {
                if (time >= track.positionKeys[i].time && 
                    time < track.positionKeys[i + 1].time) {
                    
                    float t1 = track.positionKeys[i].time;
                    float t2 = track.positionKeys[i + 1].time;
                    float factor = (time - t1) / (t2 - t1);
                    
                    return Vector3::Lerp(
                        track.positionKeys[i].position,
                        track.positionKeys[i + 1].position,
                        factor
                    );
                }
            }
            
            // If time is past all keyframes, return last
            return track.positionKeys.back().position;
        }
        
        // --------------------------------------------------------------------
        // Interpolate rotation at given time (slerp for smooth rotation)
        // --------------------------------------------------------------------
        Quaternion InterpolateRotation(const BoneAnimationTrack& track, float time) {
            if (track.rotationKeys.empty()) return Quaternion::Identity;
            if (track.rotationKeys.size() == 1) return track.rotationKeys[0].rotation;
            
            for (size_t i = 0; i < track.rotationKeys.size() - 1; i++) {
                if (time >= track.rotationKeys[i].time && 
                    time < track.rotationKeys[i + 1].time) {
                    
                    float t1 = track.rotationKeys[i].time;
                    float t2 = track.rotationKeys[i + 1].time;
                    float factor = (time - t1) / (t2 - t1);
                    
                    return Quaternion::Slerp(
                        track.rotationKeys[i].rotation,
                        track.rotationKeys[i + 1].rotation,
                        factor
                    );
                }
            }
            
            return track.rotationKeys.back().rotation;
        }
        
        // --------------------------------------------------------------------
        // Interpolate scale at given time
        // --------------------------------------------------------------------
        Vector3 InterpolateScale(const BoneAnimationTrack& track, float time) {
            if (track.scaleKeys.empty()) return Vector3::One;
            if (track.scaleKeys.size() == 1) return track.scaleKeys[0].scale;
            
            for (size_t i = 0; i < track.scaleKeys.size() - 1; i++) {
                if (time >= track.scaleKeys[i].time && 
                    time < track.scaleKeys[i + 1].time) {
                    
                    float t1 = track.scaleKeys[i].time;
                    float t2 = track.scaleKeys[i + 1].time;
                    float factor = (time - t1) / (t2 - t1);
                    
                    return Vector3::Lerp(
                        track.scaleKeys[i].scale,
                        track.scaleKeys[i + 1].scale,
                        factor
                    );
                }
            }
            
            return track.scaleKeys.back().scale;
        }
    };

} // namespace Animation
