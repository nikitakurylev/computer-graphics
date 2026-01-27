// Debug Line Vertex Shader for AABB visualization

cbuffer ConstantBuffer : register(b0)
{
    row_major float4x4 World;
    row_major float4x4 ViewProjection;
    float4 ViewPosition;
    row_major float4x4 InverseProjectionView;
    row_major float4x4 ViewInv;
    row_major float4x4 ProjInv;
    row_major float4x4 View;
    row_major float4x4 Projection;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(float4(input.position, 1.0f), ViewProjection);
    output.color = input.color;
    return output;
}