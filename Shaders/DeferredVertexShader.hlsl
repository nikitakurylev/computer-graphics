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
    float4 norm : NORMALWS;
	float2 texcoord : TEXCOORD;
    float4 world_pos : TEXCOORD1;
    float3 camera_direction : TEXCOORD2;
    float4 depth_pos : TEXCOORD3;
};

VOut main(float4 pos : POSITION, float4 norm : NORMAL, float2 texcoord : TEXCOORD)
{
	VOut output;

    output.norm = mul(float4(norm.xyz, 0), World);
    output.world_pos = mul(float4(pos.xyz, 1), World);
    output.pos = mul(float4(pos.xyz, 1), ViewProjection);
    output.depth_pos = output.pos.z;
    output.texcoord = texcoord;
    output.camera_direction = normalize(ViewPosition.xyz - output.world_pos.xyz);

	return output;
}