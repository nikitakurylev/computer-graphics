cbuffer LightBuffer : register(b0)
{
    row_major float4x4 ViewProj[4];
    float4 view_pos;
    float4 dyn_position;
    float4 dyn_color;
    float4 dyn_k;
    float4 debug;
}

Texture2D NormalMap : register(t0);
Texture2D Texture : register(t1);
Texture2D Position	: register(t2);
Texture2D Depth	: register(t3);

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

PixelShaderOutput main(VertexToPixel input) : SV_TARGET
{
    int3 sampleIndices = int3(input.pos.xy, 0);

    float4 textureColor = Texture.Load(sampleIndices);
    if (textureColor.w == 0)
        discard;
    float depth_pos = Depth.Load(sampleIndices).x;
    
    if (depth_pos.x > input.depth_pos.x)
        discard;
    
    float3 normal = NormalMap.Load(sampleIndices).xyz;

    float3 world_pos = Position.Load(sampleIndices).xyz;
    
    const float3 view_direction = normalize(view_pos.xyz - world_pos.xyz);
    
    float3 dyn;
    float3 direction = world_pos.xyz - dyn_position.xyz;
    float dist = length(direction);
    direction = normalize(direction);
    float angle = dot(input.down.xyz, direction);
    if (dist > dyn_k.x || angle < 0.70710678118f)
        discard;
    
    const float3 dyn_light_direction = normalize(dyn_position.xyz - world_pos.xyz);
    const float3 dyn_reflection_vector = normalize(reflect(dyn_light_direction, normal));
    const float3 dyn_diffuse = max(0, dot(dyn_light_direction, normal)) * textureColor.xyz;
    const float3 dyn_specular = pow(max(0, dot(-view_direction, dyn_reflection_vector)), dyn_k.y) * dyn_k.z;

    dyn = dyn_color.xyz * (dyn_diffuse + dyn_specular) / pow(dist, 2);
    
    PixelShaderOutput output;

    output.Color = float4(dyn, 1);
    output.Color.rgb *= (1 - pow(dist / dyn_k.x, 3)) * (1 - (1 - angle) / 0.29289321881);
    return output; //dist / dyn_k.x;//(1 - angle) / 0.29289321881;

}