#define SORT_SIZE	( 512 )
#if( SORT_SIZE>4096 )
	// won't work for arrays>4096
	#error due to LDS size SORT_SIZE must be 4096 or smaller
#else
#define ITEMS_PER_GROUP	( SORT_SIZE )
#endif

#define HALF_SIZE		(SORT_SIZE/2)
#define ITERATIONS		(HALF_SIZE > 1024 ? HALF_SIZE/1024 : 1)
#define NUM_THREADS		(HALF_SIZE/ITERATIONS)
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
void BitonicSortLDS(uint3 Gid : SV_GroupID,
					 uint3 DTid : SV_DispatchThreadID,
					 uint3 GTid : SV_GroupThreadID,
					 uint GI : SV_GroupIndex)
{
    int GlobalBaseIndex = (Gid.x * SORT_SIZE) + GTid.x;
    int LocalBaseIndex = GI;

    int numElementsInThreadGroup = min(SORT_SIZE, g_NumElements.x - (Gid.x * SORT_SIZE));
	
    // Load shared data
    int i;
	[unroll]
    for (i = 0; i < 2 * ITERATIONS; ++i)
    {
        if (GI + i * NUM_THREADS < numElementsInThreadGroup)
            g_LDS[LocalBaseIndex + i * NUM_THREADS] = Data[GlobalBaseIndex + i * NUM_THREADS];
    }
    GroupMemoryBarrierWithGroupSync();
    
	// Bitonic sort
    for (unsigned int nMergeSize = 2; nMergeSize <= SORT_SIZE; nMergeSize = nMergeSize * 2)
    {
        for (int nMergeSubSize = nMergeSize >> 1; nMergeSubSize > 0; nMergeSubSize = nMergeSubSize >> 1)
        {
			[unroll]
            for (i = 0; i < ITERATIONS; ++i)
            {
                int tmp_index = GI + NUM_THREADS * i;
                int index_low = tmp_index & (nMergeSubSize - 1);
                int index_high = 2 * (tmp_index - index_low);
                int index = index_high + index_low;

                unsigned int nSwapElem = nMergeSubSize == nMergeSize >> 1 ? index_high + (2 * nMergeSubSize - 1) - index_low : index_high + nMergeSubSize + index_low;
                if (nSwapElem < numElementsInThreadGroup)
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
        }
    }
    
    // Store shared data
	[unroll]
    for (i = 0; i < 2 * ITERATIONS; ++i)
    {
        if (GI + i * NUM_THREADS < numElementsInThreadGroup)
            Data[GlobalBaseIndex + i * NUM_THREADS] = g_LDS[LocalBaseIndex + i * NUM_THREADS];
    }
}