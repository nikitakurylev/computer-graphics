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
    float4 depth_pos : TEXTURE0;
    float2 texcoord : TEXCOORD;
};

PixelShader_Input VSMain(float4 pos : POSITION, float4 norm : NORMAL, float2 texcoord : TEXCOORD)
{
    PixelShader_Input output;
    output.pos = mul(float4(pos.xyz, 1), ViewProjection);
    output.depth_pos = output.pos;
    output.texcoord = texcoord;

    return output;
}

float4 PSMain(PixelShader_Input input) : SV_TARGET
{
    //float4 textureColor = diffTexture.Sample(SampleType, input.texcoord);
    //if (textureColor.a < 0.5f)
    //    discard;
    // float depthValue = 0.0001 / (1.0001 - input.depth_pos.z / input.depth_pos.w);
    float depthValue = input.depth_pos.z / input.depth_pos.w;
    return float4(depthValue, depthValue, depthValue, 1.0f);
}
