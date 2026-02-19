#pragma once
#include "Bone.h"
#include <unordered_map>
#include <memory>

// ============================================================================
// Skeleton.h
// Skeleton class managing bone hierarchy and transformations
// ============================================================================

namespace Animation {

    // ------------------------------------------------------------------------
    // Skeleton - manages the hierarchy of bones
    // 
    // Responsibilities:
    // - Store all bones in the skeleton
    // - Update bone transformations hierarchically
    // - Provide bone matrices for GPU skinning
    // - Map bone names to IDs for animation
    // ------------------------------------------------------------------------
    class Skeleton {
    public:
        Skeleton() : rootBoneId(-1) {}
        
        // --------------------------------------------------------------------
        // Add a bone to the skeleton
        // Returns the bone ID
        // --------------------------------------------------------------------
        int AddBone(const Bone& bone) {
            int boneId = static_cast<int>(bones.size());
            
            Bone newBone = bone;
            newBone.id = boneId;
            
            bones.push_back(newBone);
            boneNameToId[bone.name] = boneId;
            
            // Update parent-child relationships
            if (bone.parentId >= 0 && bone.parentId < bones.size()) {
                bones[bone.parentId].AddChild(boneId);
            }
            
            // Set root if this is the first bone without parent
            if (bone.parentId == -1 && rootBoneId == -1) {
                rootBoneId = boneId;
            }
            
            return boneId;
        }
        
        // --------------------------------------------------------------------
        // Get bone by ID
        // --------------------------------------------------------------------
        Bone* GetBone(int boneId) {
            if (boneId < 0 || boneId >= bones.size()) return nullptr;
            return &bones[boneId];
        }
        
        // --------------------------------------------------------------------
        // Get bone by name
        // --------------------------------------------------------------------
        Bone* GetBone(const std::string& name) {
            auto it = boneNameToId.find(name);
            if (it == boneNameToId.end()) return nullptr;
            return &bones[it->second];
        }
        
        // --------------------------------------------------------------------
        // Get bone ID by name
        // --------------------------------------------------------------------
        int GetBoneId(const std::string& name) const {
            auto it = boneNameToId.find(name);
            if (it == boneNameToId.end()) return -1;
            return it->second;
        }
        
        // --------------------------------------------------------------------
        // Update global transforms for all bones (hierarchically)
        // Should be called after modifying local transforms
        // --------------------------------------------------------------------
        void UpdateTransforms() {
            if (rootBoneId >= 0) {
                UpdateBoneTransform(rootBoneId, Matrix::Identity);
            }
        }
        
        // --------------------------------------------------------------------
        // Get bone matrices for GPU (final transforms)
        // Returns array of matrices ready for constant buffer
        // --------------------------------------------------------------------
        const std::vector<Matrix>& GetBoneMatrices() const {
            return boneMatrices;
        }
        
        // --------------------------------------------------------------------
        // Update bone matrices for rendering
        // Call this after UpdateTransforms()
        // --------------------------------------------------------------------
        void UpdateBoneMatrices() {
            boneMatrices.resize(bones.size());
            
            for (size_t i = 0; i < bones.size(); i++) {
                // Final transform = offset * global transform
                boneMatrices[i] = bones[i].offsetMatrix * bones[i].globalTransform;
                bones[i].finalTransform = boneMatrices[i];
            }
        }
        
        // --------------------------------------------------------------------
        // Get number of bones
        // --------------------------------------------------------------------
        size_t GetBoneCount() const {
            return bones.size();
        }
        
        // --------------------------------------------------------------------
        // Reset skeleton to bind pose
        // --------------------------------------------------------------------
        void ResetToBindPose() {
            for (auto& bone : bones) {
                bone.localTransform = Matrix::Identity;
            }
            UpdateTransforms();
            UpdateBoneMatrices();
        }
        
        // --------------------------------------------------------------------
        // Get root bone ID
        // --------------------------------------------------------------------
        int GetRootBoneId() const {
            return rootBoneId;
        }
        
        // --------------------------------------------------------------------
        // Debug: Print skeleton hierarchy
        // --------------------------------------------------------------------
        void PrintHierarchy() const {
            if (rootBoneId >= 0) {
                PrintBoneHierarchy(rootBoneId, 0);
            }
        }
        
    private:
        std::vector<Bone> bones;                        // All bones in the skeleton
        std::unordered_map<std::string, int> boneNameToId;  // Name to ID mapping
        std::vector<Matrix> boneMatrices;               // Final matrices for GPU
        int rootBoneId;                                 // Root bone ID
        
        // --------------------------------------------------------------------
        // Recursively update bone transforms
        // --------------------------------------------------------------------
        void UpdateBoneTransform(int boneId, const Matrix& parentTransform) {
            if (boneId < 0 || boneId >= bones.size()) return;
            
            Bone& bone = bones[boneId];
            
            // Global transform = parent global * local
            bone.globalTransform = bone.localTransform * parentTransform;
            
            // Update children
            for (int childId : bone.childIds) {
                UpdateBoneTransform(childId, bone.globalTransform);
            }
        }
        
        // --------------------------------------------------------------------
        // Print bone hierarchy (for debugging)
        // --------------------------------------------------------------------
        void PrintBoneHierarchy(int boneId, int depth) const {
            if (boneId < 0 || boneId >= bones.size()) return;
            
            const Bone& bone = bones[boneId];
            
            std::string indent(depth * 2, ' ');
            printf("%s[%d] %s\n", indent.c_str(), boneId, bone.name.c_str());
            
            for (int childId : bone.childIds) {
                PrintBoneHierarchy(childId, depth + 1);
            }
        }
    };

} // namespace Animation
