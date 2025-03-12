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

cbuffer ConstantBuffer : register(b0)
{
    row_major float4x4 Transform;
}

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
    
    output.pos = mul(float4(input.pos.xyz, 1), Transform);
    output.col = input.col;
	
	return output;
}

float4 PSMain(PS_IN input) : SV_Target
{

    float dist = input.texCoord.x * input.texCoord.x
               + input.texCoord.y * input.texCoord.y;
    //if (dist > 1.0f) discard;
    return input.col;
}