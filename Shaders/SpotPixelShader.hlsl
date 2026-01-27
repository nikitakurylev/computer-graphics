cbuffer LightBuffer : register(b0)
{
    row_major float4x4 ViewProj[4];
    float4 view_pos;
    float4 dyn_position;
    float4 dyn_color;
    float4 dyn_k;
    float4 debug;
}

Texture2D GBuffer0 : register(t0); // Normal.xyz, Roughness
Texture2D GBuffer1 : register(t1); // Albedo.rgb, Metallic
Texture2D GBuffer2 : register(t2); // Position.xyz, AO
Texture2D GBuffer3 : register(t3); // Depth, Mask

SamplerState Sampler	: register(s0);

struct VertexToPixel
{
    float4 pos : SV_POSITION;
    float4 depth_pos : TEXCOORD;
    float4 down : TEXCOORD1;
};

struct PixelShaderOutput
{
    float4 Color : SV_Target0;
};

static const float PI = 3.14159265f;

float DistributionGGX(float3 n, float3 h, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float nDotH = max(dot(n, h), 0.0f);
    float nDotH2 = nDotH * nDotH;
    float denom = (nDotH2 * (a2 - 1.0f) + 1.0f);
    return a2 / (PI * denom * denom + 1e-5f);
}

float GeometrySchlickGGX(float nDotV, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return nDotV / (nDotV * (1.0f - k) + k + 1e-5f);
}

float GeometrySmith(float3 n, float3 v, float3 l, float roughness)
{
    float nDotV = max(dot(n, v), 0.0f);
    float nDotL = max(dot(n, l), 0.0f);
    float ggx2 = GeometrySchlickGGX(nDotV, roughness);
    float ggx1 = GeometrySchlickGGX(nDotL, roughness);
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

PixelShaderOutput main(VertexToPixel input)
{
    int3 sampleIndices = int3(input.pos.xy, 0);

    float4 g0 = GBuffer0.Load(sampleIndices);
    float4 g1 = GBuffer1.Load(sampleIndices);
    float4 g2 = GBuffer2.Load(sampleIndices);
    float4 g3 = GBuffer3.Load(sampleIndices);

    if (g3.y == 0)
        discard;

    float depth_pos = g3.x;
    
    if (depth_pos > input.depth_pos.x)
        discard;
    
    float3 normal = normalize(g0.xyz);
    float roughness = saturate(g0.w);
    roughness = max(roughness, 0.04f);

    float3 albedo = g1.xyz;
    float metallic = saturate(g1.w);

    float3 world_pos = g2.xyz;
    
    const float3 view_direction = normalize(view_pos.xyz - world_pos.xyz);
    
    float3 direction = world_pos.xyz - dyn_position.xyz;
    float dist = length(direction);
    direction = normalize(direction);
    float angle = dot(input.down.xyz, direction);
    if (dist > dyn_k.x || angle < 0.70710678118f)
        discard;
    
    const float3 light_direction = normalize(dyn_position.xyz - world_pos.xyz);
    const float3 halfway = normalize(view_direction + light_direction);

    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);
    float3 F = FresnelSchlick(max(dot(halfway, view_direction), 0.0f), F0);
    float NDF = DistributionGGX(normal, halfway, roughness);
    float G = GeometrySmith(normal, view_direction, light_direction, roughness);

    float3 numerator = NDF * G * F;
    float denom = 4.0f * max(dot(normal, view_direction), 0.0f) * max(dot(normal, light_direction), 0.0f) + 1e-4f;
    float3 specular = numerator / denom;

    float3 kS = F;
    float3 kD = (1.0f - kS) * (1.0f - metallic);

    float NdotL = max(dot(normal, light_direction), 0.0f);
    float attenuation = 1.0f / max(dist * dist, 0.01f);
    float smooth = (1.0f - pow(dist / dyn_k.x, 3.0f)) * (1.0f - (1.0f - angle) / 0.29289321881f);
    float3 radiance = dyn_color.xyz * dyn_k.z;
    float3 direct = (kD * albedo / PI + specular) * radiance * NdotL * attenuation;
    
    PixelShaderOutput output;

    output.Color = float4(direct * smooth, 1);
    return output; //dist / dyn_k.x;//(1 - angle) / 0.29289321881;

}