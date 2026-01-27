Texture2D diffTexture : register(t0);
SamplerState SampleType : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix ViewProjection;
    float4 ViewPosition;
    matrix InverseProjectionView;
    matrix ViewInv;
    matrix ProjInv;
}

struct PixelShader_Input
{
    float4 pos : SV_POSITION;
    float4 color : TEXCOORD1;
    //float4 world_pos : TEXCOORD1;
};

PixelShader_Input VSMain(float4 pos : POSITION, float3 norm : NORMAL, float2 texcoord : TEXCOORD, float3 tangent : TANGENT, float3 bitangent : BINORMAL)
{
    PixelShader_Input output;
    output.pos = mul(float4(pos.xyz, 1), ViewProjection);
    output.color = float4(norm, 1.0f);
    //output.world_pos = mul(float4(pos.xyz, 1), World);
    
    return output;
}

float4 PSMain(PixelShader_Input input) : SV_TARGET
{
    //if(input.world_pos.y < 0)
    //    discard;
    return input.color;
}
