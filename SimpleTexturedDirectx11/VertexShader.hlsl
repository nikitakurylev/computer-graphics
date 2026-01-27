cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix ViewProjection;
    float4 ViewPosition;
    matrix InverseProjectionView;
    matrix ViewInv;
    matrix ProjInv;
}

cbuffer LightBuffer : register(b1)
{
    matrix ViewProj[4];
    float4 Distances;
    float4 direction;
    float4 color;
    float4 k;
    float4 debug;
}

struct VOut {
	float4 pos : SV_POSITION;
    float4 norm : NORMAL;
	float2 texcoord : TEXCOORD;
    float4 world_pos : TEXCOORD1;
    float4 view_pos : TEXCOORD2;
    float3 camera_direction : TEXCOORD3;
    float4 depth_pos : TEXCOORD4;
    float4 pos_in_light_view[4] : TEXCOORD5;
};

VOut main(float4 pos : POSITION, float3 norm : NORMAL, float2 texcoord : TEXCOORD, float3 tangent : TANGENT, float3 bitangent : BINORMAL)
{
	VOut output;

    output.norm = mul(float4(norm.xyz, 0), World);
    output.world_pos = mul(float4(pos.xyz, 1), World);
    output.pos = mul(float4(pos.xyz, 1), ViewProjection);
    output.depth_pos = float4(output.pos.z, 0.0f, 0.0f, 0.0f);
    output.texcoord = texcoord;
    output.view_pos = ViewPosition;
    for (int i = 0; i < 4; i++)
        output.pos_in_light_view[i] = mul(output.world_pos, ViewProj[i]);
    output.camera_direction = normalize(ViewPosition.xyz - output.world_pos.xyz);

	return output;
}