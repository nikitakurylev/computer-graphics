cbuffer LightBuffer : register(b0)
{
    row_major float4x4 ViewProj[4];
    float4 view_pos;
    float4 direction;
    float4 color;
    float4 k;
    float4 debug;
}

Texture2D Input : register(t0);

SamplerState Sampler	: register(s0);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	int3 sampleIndices = int3(input.position.xy, 0);

    float4 col = Input.Load(sampleIndices);
    col.rgb = pow(col.rgb, 1 / 2.2f);
    return col;
}