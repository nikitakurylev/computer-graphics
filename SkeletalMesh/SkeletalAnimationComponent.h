#pragma once
#include "../Component.h"
#include "AnimationClip.h"
#include "Skeleton.h"

// ============================================================================
// SkeletalAnimationComponent.h
// Component for playing skeletal animations
// ============================================================================

class SkeletalAnimationComponent : public Component {
public:
    SkeletalAnimationComponent(Animation::Skeleton* skeleton)
        : skeleton_(skeleton)
        , currentClip_(nullptr)
        , playbackTime_(0.0f)
        , isPlaying_(false)
        , loop_(true)
        , playbackSpeed_(1.0f) {
    }
    
    virtual ~SkeletalAnimationComponent() {
        // Don't delete clip or skeleton - they're owned externally
    }
    
    // ------------------------------------------------------------------------
    // Set animation clip to play
    // ------------------------------------------------------------------------
    void SetAnimationClip(Animation::AnimationClip* clip) {
        currentClip_ = clip;
        playbackTime_ = 0.0f;
    }
    
    // ------------------------------------------------------------------------
    // Play animation
    // ------------------------------------------------------------------------
    void Play() {
        isPlaying_ = true;
    }
    
    // ------------------------------------------------------------------------
    // Stop animation
    // ------------------------------------------------------------------------
    void Stop() {
        isPlaying_ = false;
        playbackTime_ = 0.0f;
    }
    
    // ------------------------------------------------------------------------
    // Pause animation
    // ------------------------------------------------------------------------
    void Pause() {
        isPlaying_ = false;
    }
    
    // ------------------------------------------------------------------------
    // Set looping
    // ------------------------------------------------------------------------
    void SetLoop(bool loop) {
        loop_ = loop;
    }
    
    // ------------------------------------------------------------------------
    // Set playback speed (1.0 = normal, 2.0 = double speed, etc.)
    // ------------------------------------------------------------------------
    void SetPlaybackSpeed(float speed) {
        playbackSpeed_ = speed;
    }
    
    // ------------------------------------------------------------------------
    // Update - advance animation and update skeleton
    // ------------------------------------------------------------------------
    void Update(float deltaTime) override {
        if (!isPlaying_ || !currentClip_ || !skeleton_) {
            return;
        }
        
        // Advance playback time
        playbackTime_ += deltaTime * playbackSpeed_;
        
        // Handle looping
        if (loop_) {
            if (playbackTime_ >= currentClip_->duration) {
                playbackTime_ = fmod(playbackTime_, currentClip_->duration);
            }
        } else {
            // Clamp to duration if not looping
            if (playbackTime_ >= currentClip_->duration) {
                playbackTime_ = currentClip_->duration;
                isPlaying_ = false;
            }
        }
        
        // Sample animation at current time
        currentClip_->Sample(playbackTime_, skeleton_);
        
        // Update skeleton transforms
        skeleton_->UpdateTransforms();
        skeleton_->UpdateBoneMatrices();
    }
    
    // ------------------------------------------------------------------------
    // Get current playback time
    // ------------------------------------------------------------------------
    float GetPlaybackTime() const {
        return playbackTime_;
    }
    
    // ------------------------------------------------------------------------
    // Check if playing
    // ------------------------------------------------------------------------
    bool IsPlaying() const {
        return isPlaying_;
    }
    
    // ------------------------------------------------------------------------
    // Get current clip
    // ------------------------------------------------------------------------
    Animation::AnimationClip* GetCurrentClip() const {
        return currentClip_;
    }
    
private:
    Animation::Skeleton* skeleton_;
    Animation::AnimationClip* currentClip_;
    float playbackTime_;
    bool isPlaying_;
    bool loop_;
    float playbackSpeed_;
};
