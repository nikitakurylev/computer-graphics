cbuffer ConstantBuffer : register(b0)
{
    row_major matrix World;
    row_major matrix View;
    float4 ViewPosition;
}

struct PixelShader_Input
{
    float4 pos : SV_POSITION;
    float4 depth_pos : TEXTURE0;
};

PixelShader_Input VSMain(float4 pos : POSITION, float4 norm : NORMAL, float2 texcoord : TEXCOORD)
{
    PixelShader_Input output;
    output.pos = mul(float4(pos.xyz, 1), View);
    output.depth_pos = output.pos;
    output.pos = output.depth_pos;

    return output;
}

float4 PSMain(PixelShader_Input input) : SV_TARGET
{
    // float depthValue = 0.0001 / (1.0001 - input.depth_pos.z / input.depth_pos.w);
    float depthValue = input.depth_pos.z / input.depth_pos.w;
    return float4(depthValue, depthValue, depthValue, 1.0f);
}
