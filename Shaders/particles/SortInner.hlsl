#define SORT_SIZE	( 512 )
#if( SORT_SIZE>2048 )
	#error
#endif

#define NUM_THREADS		(SORT_SIZE/2)
#define INVERSION		(16*2 + 8*3)

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer NumElementsCB : register(b0)
{
    int4 g_NumElements;
};

//--------------------------------------------------------------------------------------
// Structured Buffers
//--------------------------------------------------------------------------------------
RWStructuredBuffer<float2> Data : register(u0);


//--------------------------------------------------------------------------------------
// Bitonic Sort Compute Shader
//--------------------------------------------------------------------------------------
groupshared float2 g_LDS[SORT_SIZE];


[numthreads(NUM_THREADS, 1, 1)]
void BitonicInnerSort(uint3 Gid : SV_GroupID,
						uint3 DTid : SV_DispatchThreadID,
						uint3 GTid : SV_GroupThreadID,
						uint GI : SV_GroupIndex)
{
    int4 tgp;

    tgp.x = Gid.x * 256;
    tgp.y = 0;
    tgp.z = g_NumElements.x;
    tgp.w = min(512, max(0, g_NumElements.x - Gid.x * 512));

    int GlobalBaseIndex = tgp.y + tgp.x * 2 + GTid.x;
    int LocalBaseIndex = GI;
    int i;

    // Load shared data
	[unroll]
    for (i = 0; i < 2; ++i)
    {
        if (GI + i * NUM_THREADS < tgp.w)
            g_LDS[LocalBaseIndex + i * NUM_THREADS] = Data[GlobalBaseIndex + i * NUM_THREADS];
    }
    GroupMemoryBarrierWithGroupSync();

	// sort threadgroup shared memory
    for (int nMergeSubSize = SORT_SIZE >> 1; nMergeSubSize > 0; nMergeSubSize = nMergeSubSize >> 1)
    {
        int tmp_index = GI;
        int index_low = tmp_index & (nMergeSubSize - 1);
        int index_high = 2 * (tmp_index - index_low);
        int index = index_high + index_low;

        unsigned int nSwapElem = index_high + nMergeSubSize + index_low;

        if (nSwapElem < tgp.w)
        {
            float2 a = g_LDS[index];
            float2 b = g_LDS[nSwapElem];

            if (a.x > b.x)
            {
                g_LDS[index] = b;
                g_LDS[nSwapElem] = a;
            }
        }
        GroupMemoryBarrierWithGroupSync();
    }
    
    // Store shared data
	[unroll]
    for (i = 0; i < 2; ++i)
    {
        if (GI + i * NUM_THREADS < tgp.w)
            Data[GlobalBaseIndex + i * NUM_THREADS] = g_LDS[LocalBaseIndex + i * NUM_THREADS];
    }
}