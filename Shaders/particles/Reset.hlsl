struct Particle
{
    float3 positon;
    float _pad1;
    float3 velocity;
    float age;
    float4 color;
};

AppendStructuredBuffer<uint> deadListBuffer : register(u0);
RWStructuredBuffer<Particle> particleList : register(u1);

[numthreads(1024, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    deadListBuffer.Append(id.x);

    particleList[id.x] = (Particle) 0;

}