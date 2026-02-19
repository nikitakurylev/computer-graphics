#pragma once
#include <string>
#include <vector>
#include <DirectXMath.h>
#include "SimpleMath.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// ============================================================================
// Bone.h
// Bone structure for skeletal animation system
// ============================================================================

namespace Animation {

    // ------------------------------------------------------------------------
    // Bone - represents a single bone in the skeleton hierarchy
    // 
    // Each bone has:
    // - Local transform (relative to parent)
    // - Global transform (in model space)
    // - Offset matrix (bind pose inverse)
    // - Hierarchy information (parent/children)
    // ------------------------------------------------------------------------
    struct Bone {
        std::string name;               // Bone name (from FBX)
        int id;                         // Unique bone ID
        int parentId;                   // Parent bone ID (-1 for root)
        
        // Transforms
        Matrix offsetMatrix;            // Inverse bind pose matrix
        Matrix localTransform;          // Transform relative to parent
        Matrix globalTransform;         // Transform in model space
        Matrix finalTransform;          // offsetMatrix * globalTransform (for shader)
        
        // Hierarchy
        std::vector<int> childIds;      // Child bone IDs
        
        // Constructor
        Bone() : id(-1), parentId(-1) {
            offsetMatrix = Matrix::Identity;
            localTransform = Matrix::Identity;
            globalTransform = Matrix::Identity;
            finalTransform = Matrix::Identity;
        }
        
        Bone(const std::string& boneName, int boneId, int parent = -1)
            : name(boneName), id(boneId), parentId(parent) {
            offsetMatrix = Matrix::Identity;
            localTransform = Matrix::Identity;
            globalTransform = Matrix::Identity;
            finalTransform = Matrix::Identity;
        }
        
        // Add child bone
        void AddChild(int childId) {
            childIds.push_back(childId);
        }
    };

} // namespace Animation
