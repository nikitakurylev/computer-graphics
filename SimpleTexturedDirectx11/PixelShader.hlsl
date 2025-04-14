Texture2D diffTexture : register(t0);
Texture2D DirLightDepthMapTexture : register(t1);
SamplerState SampleType : register(s0);
SamplerState DepthSampleType : register(s1);

struct PointLight
{
    float4 dyn_position;
    float4 dyn_color;
    float4 dyn_k;
};

cbuffer LightBuffer : register(b1)
{
    float4 direction;
    float4 color;
    float4 k;
}

cbuffer DynamicLightBuffer : register(b2)
{
    PointLight lights[10];
}

float4 main(float4 pos : SV_POSITION, float4 norm : NORMAL, float2 texcoord : TEXCOORD0, float4 world_pos : TEXCOORD1, float4 view_pos : TEXCOORD2, float4 pos_in_light_view : TEXCOORD3, float3 camera_direction : TEXCOORD4) : SV_TARGET
{
	float4 textureColor = diffTexture.Sample(SampleType, texcoord);
	if(textureColor.a < 0.5f)
        discard;
    
    float2 projected_tex_coord;
    projected_tex_coord.x = pos_in_light_view.x / pos_in_light_view.w / 2.0f + 0.5f;
    projected_tex_coord.y = -pos_in_light_view.y / pos_in_light_view.w / 2.0f + 0.5f;
    const float depth = DirLightDepthMapTexture.Sample(DepthSampleType, projected_tex_coord).r;
    const float light_depth = pos_in_light_view.z / pos_in_light_view.w - 5e-6f;

    
    const float3 normal = norm.xyz;

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

    float4 col = float4(color.xyz * (diffuse + specular) * (light_depth < depth) + dyn + ambient, 1);
    col.rgb = pow(col.rgb, 1 / 2.2f);
    
    return col; //float4(normal, 0) / 2 + float4(0.5f, 0.5f, 0.5f, 0.0f);
}