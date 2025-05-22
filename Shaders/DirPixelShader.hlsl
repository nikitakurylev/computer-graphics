cbuffer LightBuffer : register(b0)
{
    row_major float4x4 ViewProj[4];
    float4 view_pos;
    float4 direction;
    float4 color;
    float4 k;
    float4 debug;
}

Texture2D NormalMap : register(t0);
Texture2D Texture : register(t1);
Texture2D Position	: register(t2);
Texture2D Depth	: register(t3);
Texture2D DirLightDepthMapTexture1 : register(t4);
Texture2D DirLightDepthMapTexture2 : register(t5);
Texture2D DirLightDepthMapTexture3 : register(t6);
Texture2D DirLightDepthMapTexture4 : register(t7);

SamplerState Sampler	: register(s0);
SamplerState DepthSampleType : register(s1);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
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

float4 main(VertexToPixel input) : SV_TARGET
{
	int3 sampleIndices = int3(input.position.xy, 0);

    float4 textureColor = Texture.Load(sampleIndices);
    if (textureColor.w == 0)
        return float4(0.054f, 0.149f, 0.49f, 0);
    
    float3 normal = NormalMap.Load(sampleIndices).xyz;

    float3 world_pos = Position.Load(sampleIndices).xyz;
    float depth_pos = Depth.Load(sampleIndices).x;
    float4 pos_in_light_view[4];
    for (int i = 0; i < 4; i++)
        pos_in_light_view[i] = mul(float4(world_pos, 1), ViewProj[i]);
    
    float3 depth = 0;
    float dist = abs(depth_pos);
    if (dist < 10)
    {
        float d = sampleDepthMap(pos_in_light_view[0], DirLightDepthMapTexture1);
        depth = debug.x ? float3(1, d, d) : d;
    }
    else if (dist < 30)
    {
        float d = sampleDepthMap(pos_in_light_view[1], DirLightDepthMapTexture2);
        depth = debug.x ? float3(d, d, 1) : d;
    }
    else if (dist < 60)
    {
        float d = sampleDepthMap(pos_in_light_view[2], DirLightDepthMapTexture3);
        depth = debug.x ? float3(1, d, 1) : d;
    }
    else if (dist < 500)
    {
        float d = sampleDepthMap(pos_in_light_view[3], DirLightDepthMapTexture4);
        depth = debug.x ? float3(1, d, d) : d;
    }
    else
    {
        depth = 1;
    }

    const float3 view_direction = normalize(view_pos.xyz - world_pos.xyz);
    const float3 light_direction = -normalize(direction.xyz);
    const float3 reflection_vector = normalize(reflect(light_direction, normal));
    
    const float3 diffuse = max(0, dot(light_direction, normal)) * textureColor.xyz;
    const float3 ambient = textureColor.xyz * float3(0.529f, 0.808f, 0.922f) * k.x;
    const float3 specular = pow(max(0, dot(-view_direction, reflection_vector)), k.y) * k.z;

    float4 col = float4(color.xyz * (diffuse + specular) * depth + ambient * (1 - debug.x), 1);
    col.rgb = pow(col.rgb, 1 / 2.2f);
    return col;
}