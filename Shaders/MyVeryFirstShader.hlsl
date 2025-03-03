struct VS_IN
{
	float4 pos : POSITION0;
	float4 col : COLOR0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
 	float4 col : COLOR;
};

struct offsetColor {
	float4 offset;
	float4 color;
};

cbuffer ConstBuf : register(b0) {
	offsetColor ConstData;
};

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.pos = float4(input.pos.xyz + ConstData.offset.xyz, 1.0f);
	output.col = input.col;// * ConstData.color;
	
	return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
	float4 col = input.col; //* ConstData.color;
	return col;
}