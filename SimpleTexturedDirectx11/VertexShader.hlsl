cbuffer ConstantBuffer : register(b0)
{
    row_major matrix World;
    row_major matrix View;
    float4 ViewPosition;
}

struct VOut {
	float4 pos : SV_POSITION;
    float4 norm : NORMAL;
	float2 texcoord : TEXCOORD;
    float4 world_pos : TEXCOORD1;
    float4 view_pos : TEXCOORD2;
};

VOut main(float4 pos : POSITION, float4 norm : NORMAL, float2 texcoord : TEXCOORD)
{
	VOut output;

    output.norm = mul(float4(norm.xyz, 0), World);
    output.world_pos = mul(float4(pos.xyz, 1), World);
    output.pos = mul(float4(pos.xyz, 1), View);
	output.texcoord = texcoord;
    output.view_pos = ViewPosition;

	return output;
}