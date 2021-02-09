cbuffer cbSettings : register(b0)
{
    int gBlurRadius;
    
    float w0;
    float w1;
    float w2;
    float w3;
    float w4;
    float w5;
    float w6;
    float w7;
    float w8;
    float w9;
    float w10;
    float cbPad0;
}

static const int gMaxBlurRadius = 5;

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define NumThreads (256)
#define CacheSize  (NumThreads + 2 * gMaxBlurRadius)

groupshared float4 gCache[CacheSize];

[numthreads(NumThreads, 1, 1)]
void HorizontalBlurCS(int3 GroupThreadID : SV_GroupThreadID, int3 DispatchThreadID : SV_DispatchThreadID)
{
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    if (GroupThreadID.x < gBlurRadius)
    {
        int x = max(DispatchThreadID.x - gBlurRadius, 0);
        gCache[GroupThreadID.x] = gInput[int2(x, DispatchThreadID.y)];
    }
    if (GroupThreadID.x >= NumThreads - gBlurRadius)
    {
        int x = min(DispatchThreadID.x + gBlurRadius, gInput.Length.x - 1);
        gCache[GroupThreadID.x + 2 * gBlurRadius] = gInput[int2(x, DispatchThreadID.y)];
    }
    gCache[GroupThreadID.x + gBlurRadius] = gInput[min(DispatchThreadID.xy, gInput.Length.xy - 1)];
    GroupMemoryBarrierWithGroupSync();
    
    float4 blurColor = 0.0;
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int cacheIndex = GroupThreadID.x + gBlurRadius + i;
        blurColor += weights[i + gBlurRadius] * gCache[cacheIndex];
    }
    gOutput[DispatchThreadID.xy] = blurColor;
}

[numthreads(1, NumThreads, 1)]
void VerticalBlurCS(int3 GroupThreadID : SV_GroupThreadID, int3 DispatchThreadID : SV_DispatchThreadID)
{
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    if (GroupThreadID.y < gBlurRadius)
    {
        int y = max(DispatchThreadID.y - gBlurRadius, 0);
        gCache[GroupThreadID.y] = gInput[int2(DispatchThreadID.x, y)];
    }
    if (GroupThreadID.y >= NumThreads - gBlurRadius)
    {
        int y = min(DispatchThreadID.y + gBlurRadius, gInput.Length.y - 1);
        gCache[GroupThreadID.y + 2 * gBlurRadius] = gInput[int2(DispatchThreadID.x, y)];
    }
    gCache[GroupThreadID.y + gBlurRadius] = gInput[min(DispatchThreadID.xy, gInput.Length.xy - 1)];
    GroupMemoryBarrierWithGroupSync();
    
    float4 blurColor = 0.0;
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int cacheIndex = GroupThreadID.y + gBlurRadius + i;
        blurColor += weights[i + gBlurRadius] * gCache[cacheIndex];
    }
    gOutput[DispatchThreadID.xy] = blurColor;
}