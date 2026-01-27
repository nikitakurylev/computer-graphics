cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix ViewProjection;
    float4 ViewPosition;
    matrix InverseProjectionView;
    matrix ViewInv;
    matrix ProjInv;
}

struct VOut {
	float4 pos : SV_POSITION;
    float3 norm : NORMALWS;
	float2 texcoord : TEXCOORD;
    float4 world_pos : TEXCOORD1;
    float3 camera_direction : TEXCOORD2;
    float4 depth_pos : TEXCOORD3;
    float3 tangent_ws : TEXCOORD4;
    float3 bitangent_ws : TEXCOORD5;
};

VOut main(float4 pos : POSITION, float3 norm : NORMAL, float2 texcoord : TEXCOORD, float3 tangent : TANGENT, float3 bitangent : BINORMAL)
{
	VOut output;

    output.norm = mul(float4(norm.xyz, 0), World).xyz;
    output.world_pos = mul(float4(pos.xyz, 1), World);
    output.pos = mul(float4(pos.xyz, 1), ViewProjection);
    output.depth_pos = float4(output.pos.z, 0.0f, 0.0f, 0.0f);
    output.texcoord = texcoord;
    output.camera_direction = normalize(ViewPosition.xyz - output.world_pos.xyz);
    output.tangent_ws = mul(float4(tangent.xyz, 0), World).xyz;
    output.bitangent_ws = mul(float4(bitangent.xyz, 0), World).xyz;

	return output;
}