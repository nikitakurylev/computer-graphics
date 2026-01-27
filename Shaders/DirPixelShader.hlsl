cbuffer LightBuffer : register(b0)
{
    row_major float4x4 ViewProj[4];
    float4 view_pos;
    float4 direction;
    float4 color;
    float4 k;
    float4 debug;
}

Texture2D GBuffer0 : register(t0); // Normal.xyz, Roughness
Texture2D GBuffer1 : register(t1); // Albedo.rgb, Metallic
Texture2D GBuffer2 : register(t2); // Position.xyz, AO
Texture2D GBuffer3 : register(t3); // Depth, Mask
Texture2D DirLightDepthMapTexture1 : register(t4);
Texture2D DirLightDepthMapTexture2 : register(t5);
Texture2D DirLightDepthMapTexture3 : register(t6);
Texture2D DirLightDepthMapTexture4 : register(t7);
TextureCube EnvironmentMap : register(t8);

SamplerState Sampler	: register(s0);
SamplerState DepthSampleType : register(s1);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
};

struct PixelShaderOutput
{
    float4 Color : SV_Target0;
};

float sampleDepthMap(float4 pos_in_light_view, Texture2D depthTexture)
{
    float light_depth = pos_in_light_view.z / pos_in_light_view.w - 5e-4f;
    float3 projected_tex_coord = pos_in_light_view.xyz / pos_in_light_view.w / 2.0f + 0.5f;
    projected_tex_coord.y = 1 - projected_tex_coord.y;
    
    float shadow = 0.0;
    float width, height;
    depthTexture.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = depthTexture.Sample(DepthSampleType, float2(projected_tex_coord.xy + float2(x, y) * texelSize)).r;
            shadow += light_depth > pcfDepth;
        }
    }
    shadow /= 9.0f;
    
    return 1 - shadow;
}

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

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float3 oneMinusRoughness = float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness);
    return F0 + (max(oneMinusRoughness, F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

PixelShaderOutput main(VertexToPixel input)
{
    PixelShaderOutput output;
    int3 sampleIndices = int3(input.position.xy, 0);

    float4 g0 = GBuffer0.Load(sampleIndices);
    float4 g1 = GBuffer1.Load(sampleIndices);
    float4 g2 = GBuffer2.Load(sampleIndices);
    float4 g3 = GBuffer3.Load(sampleIndices);

    if (g3.y == 0)
    {
        output.Color = float4(0.054f, 0.149f, 0.49f, 0);
        return output;
    }
    
    float3 normal = normalize(g0.xyz);
    float roughness = saturate(g0.w);
    roughness = max(roughness, 0.04f);

    float3 albedo = g1.xyz;
    float metallic = saturate(g1.w);

    float3 world_pos = g2.xyz;
    float ao = saturate(g2.w);

    float depth_pos = g3.x;
    float4 pos_in_light_view[4];
    for (int i = 0; i < 4; i++)
        pos_in_light_view[i] = mul(float4(world_pos, 1), ViewProj[i]);
    
    float3 depthColor = float3(1.0f, 1.0f, 1.0f);
    float dist = abs(depth_pos);
    if (dist < 10)
    {
        float d = sampleDepthMap(pos_in_light_view[0], DirLightDepthMapTexture1);
        depthColor = debug.x ? float3(1, d, d) : float3(d, d, d);
    }
    else if (dist < 30)
    {
        float d = sampleDepthMap(pos_in_light_view[1], DirLightDepthMapTexture2);
        depthColor = debug.x ? float3(d, d, 1) : float3(d, d, d);
    }
    else if (dist < 60)
    {
        float d = sampleDepthMap(pos_in_light_view[2], DirLightDepthMapTexture3);
        depthColor = debug.x ? float3(1, d, 1) : float3(d, d, d);
    }
    else if (dist < 500)
    {
        float d = sampleDepthMap(pos_in_light_view[3], DirLightDepthMapTexture4);
        depthColor = debug.x ? float3(1, d, d) : float3(d, d, d);
    }
    else
    {
        depthColor = float3(1.0f, 1.0f, 1.0f);
    }

    const float3 view_direction = normalize(view_pos.xyz - world_pos.xyz);
    const float3 light_direction = -normalize(direction.xyz);
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
    float3 radiance = color.xyz;
    float3 direct = (kD * albedo / PI + specular) * radiance * NdotL;

    float3 F_ibl = FresnelSchlickRoughness(max(dot(normal, view_direction), 0.0f), F0, roughness);
    float3 kS_ibl = F_ibl;
    float3 kD_ibl = (1.0f - kS_ibl) * (1.0f - metallic);
    float3 envDiffuse = EnvironmentMap.SampleLevel(Sampler, normal, 0).rgb;
    float3 reflection = reflect(-view_direction, normal);
    float3 envSpec = EnvironmentMap.SampleLevel(Sampler, reflection, 0).rgb;
    float3 diffuseIBL = envDiffuse * albedo * kD_ibl;
    float3 specularIBL = envSpec * kS_ibl * (1.0f - roughness);
    float3 ambient = (diffuseIBL + specularIBL) * k.x * ao;

    float3 finalColor = direct * depthColor + ambient * (1 - debug.x);

    finalColor = pow(saturate(finalColor), 1.0f / 2.2f);
    output.Color = float4(finalColor, 1);
    return output;
}