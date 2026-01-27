Texture2D AlbedoTexture : register(t0);
Texture2D AOTexture : register(t1);
Texture2D MetallicTexture : register(t2);
Texture2D RoughnessTexture : register(t3);
Texture2D NormalTexture : register(t4);
SamplerState SampleType : register(s0);

cbuffer MaterialBuffer : register(b2)
{
    float4 baseColorFactor;
    float4 materialParams; // x=metallic, y=roughness, z=ao, w=unused
}

struct PixelShaderOutput
{
    float4 Normal : SV_Target0; //Normal map
    float4 Diffuse : SV_Target1; //Color
    float4 Position : SV_Target2;
    float4 Depth : SV_Target3;
};

PixelShaderOutput main(float4 pos : SV_POSITION, float3 norm : NORMALWS, float2 texcoord : TEXCOORD0, float4 world_pos : TEXCOORD1, float3 camera_direction : TEXCOORD2, float4 depth_pos : TEXCOORD3, float3 tangent_ws : TEXCOORD4, float3 bitangent_ws : TEXCOORD5)
{
    PixelShaderOutput output;
    
	float4 baseColor = AlbedoTexture.Sample(SampleType, texcoord) * baseColorFactor;
	if(baseColor.a < 0.5f)
        discard;

    float ao = AOTexture.Sample(SampleType, texcoord).r * materialParams.z;
    float metallic = MetallicTexture.Sample(SampleType, texcoord).r * materialParams.x;
    float roughness = RoughnessTexture.Sample(SampleType, texcoord).r * materialParams.y;
    ao = saturate(ao);
    metallic = saturate(metallic);
    roughness = saturate(roughness);
    
    float3 N = normalize(norm.xyz);
    float3 T = normalize(tangent_ws);
    float3 B = normalize(bitangent_ws);
    float3 tangentNormal = NormalTexture.Sample(SampleType, texcoord).xyz * 2.0f - 1.0f;
    float3 mappedNormal = N;
    if (dot(T, T) > 1e-4f && dot(B, B) > 1e-4f) {
        float3x3 TBN = float3x3(T, B, N);
        mappedNormal = normalize(mul(tangentNormal, TBN));
    }

    output.Normal = float4(mappedNormal, roughness);
    output.Diffuse = float4(baseColor.rgb, metallic);
    output.Position = float4(world_pos.xyz, ao);
    output.Depth = float4(depth_pos.x, 1.0f, 0.0f, 0.0f);
    
    return output;
}