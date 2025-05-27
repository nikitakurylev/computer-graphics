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
VOut main(float4 pos : POSITION, float4 norm : NORMAL, float2 texcoord : TEXCOORD)
{
    VOut output;

    output.pos = mul(float4(pos.xyz, 1), ViewProjection);
    output.depth_pos = output.pos.z;

    return output;
}