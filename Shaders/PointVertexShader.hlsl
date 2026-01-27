cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix ViewProjection;
    float4 ViewPosition;
    matrix InverseProjectionView;
    matrix ViewInv;
    matrix ProjInv;
}

struct VOut
{
    float4 pos : SV_POSITION;
    float4 depth_pos : TEXCOORD;
};

// The entry point for our vertex shader
VOut main(float4 pos : POSITION, float3 norm : NORMAL, float2 texcoord : TEXCOORD, float3 tangent : TANGENT, float3 bitangent : BINORMAL)
{
    VOut output;

    output.pos = mul(float4(pos.xyz, 1), ViewProjection);
    output.depth_pos = float4(output.pos.z, 0.0f, 0.0f, 0.0f);

    return output;
}