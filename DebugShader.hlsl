Texture2D diffTexture : register(t0);
SamplerState SampleType : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    row_major matrix World;
    row_major matrix View;
    float4 ViewPosition;
}

struct PixelShader_Input
{
    float4 pos : SV_POSITION;
    float4 color : TEXCOORD1;
    //float4 world_pos : TEXCOORD1;
};

PixelShader_Input VSMain(float4 pos : POSITION, float4 norm : NORMAL, float2 texcoord : TEXCOORD)
{
    PixelShader_Input output;
    output.pos = mul(float4(pos.xyz, 1), View);
    output.color = norm;
    //output.world_pos = mul(float4(pos.xyz, 1), World);
    
    return output;
}

float4 PSMain(PixelShader_Input input) : SV_TARGET
{
    //if(input.world_pos.y < 0)
    //    discard;
    return input.color;
}
