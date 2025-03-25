cbuffer ConstantBuffer : register(b0)
{
    row_major matrix Transform;
}

struct VOut {
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

VOut main(float4 pos : POSITION, float2 texcoord : TEXCOORD)
{
	VOut output;

    output.pos = mul(pos, Transform);
	output.texcoord = texcoord;

	return output;
}