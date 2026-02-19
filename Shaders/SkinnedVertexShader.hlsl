// ============================================================================
// SkinnedVertexShader.hlsl
// Vertex shader for skeletal animation with GPU skinning
// ============================================================================

#define MAX_BONES 128

// Constant buffers
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix ViewProjection;
    float4 ViewPosition;
    matrix InverseProjectionView;
    matrix ViewInv;
    matrix ProjInv;
    matrix View;
    matrix Projection;
}

// Bone matrices for skinning
cbuffer BoneBuffer : register(b1)
{
    matrix BoneMatrices[MAX_BONES];
}

// Input vertex structure (from vertex buffer)
struct VIn {
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
    uint4 boneIds : BLENDINDICES;      // Up to 4 bones per vertex
    float4 weights : BLENDWEIGHT;       // Bone weights (must sum to 1.0)
};

// Output vertex structure (to pixel shader)
struct VOut {
    float4 pos : SV_POSITION;
    float3 norm : NORMALWS;
    float2 texcoord : TEXCOORD;
    float4 world_pos : TEXCOORD1;
    float3 camera_direction : TEXCOORD2;
    float4 depth_pos : TEXCOORD3;
    float3 tangent_ws : TEXCOORD4;
    float3 bitangent_ws : TEXCOORD5;
};

VOut main(VIn input)
{
    VOut output;
    
    // ========================================================================
    // GPU SKINNING - Calculate deformed position and normal
    // ========================================================================
    
    float4 skinnedPos = float4(0, 0, 0, 0);
    float3 skinnedNorm = float3(0, 0, 0);
    float3 skinnedTangent = float3(0, 0, 0);
    float3 skinnedBitangent = float3(0, 0, 0);
    
    // Apply transformation from each influencing bone
    [unroll]
    for (int i = 0; i < 4; i++) {
        float weight = input.weights[i];
        
        if (weight > 0.0) {
            uint boneIndex = input.boneIds[i];
            matrix boneTransform = BoneMatrices[boneIndex];
            
            // Transform position
            skinnedPos += mul(float4(input.pos, 1.0), boneTransform) * weight;
            
            // Transform normal
            skinnedNorm += mul(float4(input.norm, 0.0), boneTransform).xyz * weight;
            
            // Transform tangent
            skinnedTangent += mul(float4(input.tangent, 0.0), boneTransform).xyz * weight;
            
            // Transform bitangent
            skinnedBitangent += mul(float4(input.bitangent, 0.0), boneTransform).xyz * weight;
        }
    }
    
    // Normalize vectors
    skinnedNorm = normalize(skinnedNorm);
    skinnedTangent = normalize(skinnedTangent);
    skinnedBitangent = normalize(skinnedBitangent);
    
    // ========================================================================
    // Transform to world space and clip space
    // ========================================================================
    
    // World space normal
    output.norm = mul(float4(skinnedNorm, 0), World).xyz;
    output.norm = normalize(output.norm);
    
    // World position
    output.world_pos = mul(skinnedPos, World);
    
    // Clip space position
    output.pos = mul(skinnedPos, mul(World, ViewProjection));
    
    // Depth
    output.depth_pos = float4(output.pos.z, 0.0f, 0.0f, 0.0f);
    
    // Texture coordinates (pass through)
    output.texcoord = input.texcoord;
    
    // Camera direction
    output.camera_direction = normalize(ViewPosition.xyz - output.world_pos.xyz);
    
    // Tangent space (for normal mapping)
    output.tangent_ws = mul(float4(skinnedTangent, 0), World).xyz;
    output.tangent_ws = normalize(output.tangent_ws);
    
    output.bitangent_ws = mul(float4(skinnedBitangent, 0), World).xyz;
    output.bitangent_ws = normalize(output.bitangent_ws);
    
    return output;
}
