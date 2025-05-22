Texture2D diffTexture : register(t0);
SamplerState SampleType : register(s0);

struct PixelShaderOutput
{
    float4 Normal : SV_Target0; //Normal map
    float4 Diffuse : SV_Target1; //Color
    float4 Position : SV_Target2;
    float4 Depth : SV_Target3;
};

PixelShaderOutput main(float4 pos : SV_POSITION, float4 norm : NORMALWS, float2 texcoord : TEXCOORD0, float4 world_pos : TEXCOORD1, float3 camera_direction : TEXCOORD2, float4 depth_pos : TEXCOORD3) : SV_TARGET
{
    PixelShaderOutput output;
    
	float4 textureColor = diffTexture.Sample(SampleType, texcoord);
	if(textureColor.a < 0.5f)
        discard;
    
    output.Normal = float4(normalize(norm.xyz), 1.0f);
    output.Diffuse = textureColor;
    output.Position = world_pos;
    output.Depth = depth_pos;
    
    return output;
}