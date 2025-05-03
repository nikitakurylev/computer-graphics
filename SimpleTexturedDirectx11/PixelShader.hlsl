Texture2D diffTexture : register(t0);
Texture2D DirLightDepthMapTexture1 : register(t1);
Texture2D DirLightDepthMapTexture2 : register(t2);
Texture2D DirLightDepthMapTexture3 : register(t3);
Texture2D DirLightDepthMapTexture4 : register(t4);
SamplerState SampleType : register(s0);
SamplerState DepthSampleType : register(s1);

struct PointLight
{
    float4 dyn_position;
    float4 dyn_color;
    float4 dyn_k;
};

cbuffer LightBuffer : register(b0)
{
    row_major float4x4 ViewProj[4];
    float4 Distances;
    float4 direction;
    float4 color;
    float4 k;
}

cbuffer DynamicLightBuffer : register(b1)
{
    PointLight lights[10];
}

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

float4 main(float4 pos : SV_POSITION, float4 norm : NORMAL, float2 texcoord : TEXCOORD0, float4 world_pos : TEXCOORD1, float4 view_pos : TEXCOORD2, float3 camera_direction : TEXCOORD3, float4 depth_pos : TEXCOORD4, float4 pos_in_light_view[4] : TEXCOORD5) : SV_TARGET
{
	float4 textureColor = diffTexture.Sample(SampleType, texcoord);
	if(textureColor.a < 0.5f)
        discard;
    
    float depth = 0;
    float dist = abs(depth_pos);
    if (dist < 10)
    {
        depth = sampleDepthMap(pos_in_light_view[0], DirLightDepthMapTexture1);
    } 
    else if (dist < 30)
    {
        depth = sampleDepthMap(pos_in_light_view[1], DirLightDepthMapTexture2);
    } 
    else if (dist < 60)
    {
        depth = sampleDepthMap(pos_in_light_view[2], DirLightDepthMapTexture3);
    }
    else if (dist < 500)
    {
        depth = sampleDepthMap(pos_in_light_view[3], DirLightDepthMapTexture4);
    }
    else
    {
        depth = 1;
    }
    
    const float3 normal = normalize(norm.xyz);

    const float3 view_direction = normalize(view_pos.xyz - world_pos.xyz);
    const float3 light_direction = -normalize(direction.xyz);
    const float3 reflection_vector = normalize(reflect(light_direction, normal));

    
    float3 dyn;
    
    for (int i = 0; i < 10; i++)
    {
        const float3 dyn_light_direction = normalize(lights[i].dyn_position.xyz - world_pos.xyz);
        const float3 dyn_reflection_vector = normalize(reflect(dyn_light_direction, normal));
        const float3 dyn_diffuse = max(0, dot(dyn_light_direction, normal)) * textureColor.xyz;
        const float3 dyn_specular = pow(max(0, dot(-view_direction, dyn_reflection_vector)), lights[i].dyn_k.y) * lights[i].dyn_k.z;
        dyn += lights[i].dyn_color.xyz * (dyn_diffuse + dyn_specular) / pow(distance(lights[i].dyn_position.xyz, world_pos.xyz), 2);
    }
    
    const float3 diffuse = max(0, dot(light_direction, normal)) * textureColor.xyz;
    const float3 ambient = textureColor * float4(0.529f, 0.808f, 0.922f, 1.0f) * k.x;
    const float3 specular = pow(max(0, dot(-view_direction, reflection_vector)), k.y) * k.z;

    float4 col = float4(color.xyz * (diffuse + specular) * depth + dyn + ambient, 1);
    col.rgb = pow(col.rgb, 1 / 2.2f);
    
    float3 asda = pos_in_light_view[0].xyz / pos_in_light_view[0].w / 2.0f + 0.5;
    return col; //abs(asda.x + asda.y) < 2; //float4(dist < 20, dist < 40, dist < 60, 1); //float4(normal, 0) / 2 + float4(0.5f, 0.5f, 0.5f, 0.0f);
}