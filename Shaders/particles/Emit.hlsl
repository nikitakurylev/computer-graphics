struct Particle
{
    float3 positon;
    float _pad1;
    float3 velocity;
    float age;
    float4 color;
};

struct EmitterProperties
{
    float3 position;
    int maxNumToEmit;
    float particleLifeSpan;
};

cbuffer FrameTimeCB : register(b1)
{
    float4 g_frameTime;
};

cbuffer EmitterConstantBuffer : register(b2)
{
    EmitterProperties emitterProp;
};

cbuffer DeadListCount : register(b3)
{
    uint deadNumParticles;
    uint3 DeadListCount_pad;
};

Texture2D g_RandomBuffer : register(t0);
SamplerState g_samWrapLinear : register(s0);

RWStructuredBuffer<Particle> particleBuffer : register(u0);
ConsumeStructuredBuffer<uint> deadListBuffer : register(u1);

static float W = 1.0;
static float H = 1.0;

[numthreads(1024, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x < deadNumParticles && DTid.x < emitterProp.maxNumToEmit)
    {
        uint index = deadListBuffer.Consume();
        Particle particle = (Particle) 0;

        float2 uv = float2(index / 1024.0, index / 1024);
        float3 randomValues0 = g_RandomBuffer.SampleLevel(g_samWrapLinear, uv, 0).xyz;

        float2 uv2 = float2((index + 1) / 1024.0, g_frameTime.x * 1024);
        float3 randomValues1 = g_RandomBuffer.SampleLevel(g_samWrapLinear, uv2, 0).xyz;

        particle.positon = emitterProp.position.xyz;
        particle.velocity.y = 2 + randomValues0.b * 0.1;
        particle.velocity += float3(W, 0.0, 0.0) * randomValues0.r + float3(0.0, 0.0, H) * randomValues0.g;
        particle.age = emitterProp.particleLifeSpan + randomValues0.b * emitterProp.particleLifeSpan;
        particle.color = float4(0.25 + abs(randomValues0.r), 0.25 + abs(randomValues0.g), 0.25 + abs(randomValues0.b), 1);


        particleBuffer[index] = particle;
    }
}