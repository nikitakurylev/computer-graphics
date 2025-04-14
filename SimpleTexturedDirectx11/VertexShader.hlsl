cbuffer ConstantBuffer : register(b0)
{
    row_major matrix World;
    row_major matrix View;
    float4 ViewPosition;
}

cbuffer LightBuffer : register(b1)
{
    row_major matrix LightWorld;
    row_major matrix LightView;
    float4 CameraPosition;
}

struct VOut {
	float4 pos : SV_POSITION;
    float4 norm : NORMAL;
	float2 texcoord : TEXCOORD;
    float4 world_pos : TEXCOORD1;
    float4 view_pos : TEXCOORD2;
    float4 pos_in_light_view : TEXCOORD3;
    float3 camera_direction : TEXCOORD4;
};

VOut main(float4 pos : POSITION, float4 norm : NORMAL, float2 texcoord : TEXCOORD)
{
	VOut output;

    output.norm = mul(float4(norm.xyz, 0), World);
    output.world_pos = mul(float4(pos.xyz, 1), World);
    output.pos = mul(float4(pos.xyz, 1), View);
	output.texcoord = texcoord;
    output.view_pos = ViewPosition;
    output.pos_in_light_view = mul(output.world_pos, LightView);
    output.camera_direction = normalize(CameraPosition.xyz - output.world_pos.xyz);

	return output;
}