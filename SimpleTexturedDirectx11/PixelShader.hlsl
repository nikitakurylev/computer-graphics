Texture2D diffTexture : register(t0);
SamplerState SampleType : register(s0);

cbuffer LightBuffer : register(b1)
{
    float4 direction;
    float4 color;
    float4 k;
}

cbuffer DynamicLightBuffer : register(b2)
{
    float4 dyn_position;
    float4 dyn_color;
    float4 dyn_k;
}

float4 main(float4 pos : SV_POSITION, float4 norm : NORMAL, float2 texcoord : TEXCOORD0, float4 world_pos : TEXCOORD1, float4 view_pos : TEXCOORD2) : SV_TARGET
{
	float4 textureColor = diffTexture.Sample(SampleType, texcoord);
	if(textureColor.a < 0.5f)
        discard;
    
    const float3 normal = norm.xyz;

    const float3 view_direction = normalize(view_pos.xyz - world_pos.xyz);
    const float3 light_direction = -direction.xyz;
    const float3 dyn_light_direction = normalize(dyn_position.xyz - world_pos.xyz);
    const float3 reflection_vector = normalize(reflect(light_direction, normal));
    const float3 dyn_reflection_vector = normalize(reflect(dyn_light_direction, normal));

    const float3 diffuse = max(0, dot(light_direction, normal)) * textureColor.xyz;
    const float3 dyn_diffuse = max(0, dot(dyn_light_direction, normal)) * textureColor.xyz;
    const float3 ambient = textureColor * float4(0.529f, 0.808f, 0.922f, 1.0f) * k.x;
    const float3 specular = pow(max(0, dot(-view_direction, reflection_vector)), k.y) * k.z;
    const float3 dyn_specular = pow(max(0, dot(-view_direction, dyn_reflection_vector)), dyn_k.y) * dyn_k.z;

    float4 col = float4(color.xyz * (diffuse + specular)
                        + dyn_color * (dyn_diffuse + dyn_specular) / pow(
                            distance(dyn_position.xyz, world_pos.xyz), 2) + ambient, 1);
    col.rgb = pow(col.rgb, 1 / 2.2f);
    
    return col; //float4(normal, 0) / 2 + float4(0.5f, 0.5f, 0.5f, 0.0f);

}