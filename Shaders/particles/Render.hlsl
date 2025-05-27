struct Particle
{
    float3 positon;
    float _pad1;
    float3 velocity;
    float age;
    float4 color;
};

struct ParticleIndexElement
{
    float distance;
    float index;
};

StructuredBuffer<Particle> particles : register(t0);
StructuredBuffer<ParticleIndexElement> indexBuffer : register(t1);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix ViewProjection;
    float4 ViewPosition;
    matrix InverseProjectionView;
    matrix ViewInv;
    matrix ProjInv;
    matrix View;
    matrix Projection;
}

cbuffer aliveListCountConstantBuffer : register(b3)
{
    uint aliveNumParticles;
    uint3 aliveListPadding;
};

struct VertexInput
{
    uint VertexID : SV_VertexID;
};

struct PixelInput
{
    float4 pos : SV_POSITION;
    float4 norm : NORMALWS;
    float2 texcoord : TEXCOORD;
    float4 world_pos : TEXCOORD1;
    float3 camera_direction : TEXCOORD2;
    float4 depth_pos : TEXCOORD3;
};

struct PixelOutput
{
    float4 Normal : SV_Target0; //Normal map
    float4 Diffuse : SV_Target1; //Color
    float4 Position : SV_Target2;
    float4 Depth : SV_Target3;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output = (PixelInput) 0;

    uint index = indexBuffer[aliveNumParticles - input.VertexID - 1].index;
    
    Particle particle = particles[index];

    
    //output.norm = mul(float4(norm.xyz, 0), World);
    output.world_pos = mul(float4(particle.positon, 1), World);
    output.pos = mul(float4(particle.positon, 1), ViewProjection);
    output.depth_pos = output.pos.z;
    //output.view_pos = ViewPosition;
    
    output.texcoord = 0;
    //output.Color = particle.color.xyz;

    return output;
}

/*Texture2D particleTexture : register(t0);

SamplerState samClampLinear : register(s0);

PixelOutput PSMain(PixelInput input)
{
    PixelOutput output = (PixelOutput) 0;

    float3 particle = input.Color * particleTexture.Sample(samClampLinear, input.UV).xyz;
    output.Color = float4(particle, 1);
	
    return output;
}*/

PixelInput _offsetNprojected(PixelInput data, float2 offset, float2 uv)
{
    float aspect = Projection[1][1] / Projection[0][0];
    data.pos.xy += float2(offset.x, offset.y * aspect);
    data.texcoord = uv;
    data.norm = mul(-mul(float4(-offset.x, -offset.y, 1, 1), InverseProjectionView), World);
    float3 right = float3(
    View[0][0], 
    View[1][0],
    View[2][0]);
    float3 up = float3(
    View[0][1],
    View[1][1],
    View[2][1]);
    data.world_pos += float4(right * offset.x + up * offset.y, 0);

    return data;
}

[maxvertexcount(4)]
void GSMain(point PixelInput input[1], inout TriangleStream<PixelInput> stream)
{
    PixelInput pointOut = input[0];
	
    const float size = 0.05f;

    stream.Append(_offsetNprojected(pointOut, float2(-1, -1) * size, float2(0, 0)));
    stream.Append(_offsetNprojected(pointOut, float2(1, -1) * size, float2(1, 0)));
    stream.Append(_offsetNprojected(pointOut, float2(-1, 1) * size, float2(0, 1)));
    stream.Append(_offsetNprojected(pointOut, float2(1, 1) * size, float2(1, 1)));

    stream.RestartStrip();
}