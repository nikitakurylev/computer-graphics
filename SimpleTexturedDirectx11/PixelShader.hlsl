Texture2D diffTexture : register(t0);
SamplerState SampleType : register(s0);

float4 main(float4 pos : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 textureColor = diffTexture.Sample(SampleType, texcoord);
	if(textureColor.a < 0.5f)
        discard;
	return textureColor;
}