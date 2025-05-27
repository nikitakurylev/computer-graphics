RWBuffer<uint> g_DispatchArgs : register(u0);

cbuffer NumElementsCB : register(b0)
{
    int4 g_NumElements;
};


[numthreads(1, 1, 1)]
void InitDispatchArgs(uint3 id : SV_DispatchThreadID)
{
    g_DispatchArgs[0] = ((g_NumElements - 1) >> 9) + 1;
    g_DispatchArgs[1] = 1;
    g_DispatchArgs[2] = 1;
    g_DispatchArgs[3] = 0;
}