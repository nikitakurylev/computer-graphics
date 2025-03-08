struct VS_IN
{
	float4 pos : POSITION0;
	float4 col : COLOR0;
    float2 texCoord : TEXCOORD0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
 	float4 col : COLOR;
    float2 texCoord : TEXCOORD0;
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
	output.texCoord = input.texCoord;// * ConstData.color;
	
	return output;
}

float4 PSMain(PS_IN input) : SV_Target
{

    float dist = input.texCoord.x * input.texCoord.x
               + input.texCoord.y * input.texCoord.y;
    if (dist > 1.0f) discard;
    return input.col;
}