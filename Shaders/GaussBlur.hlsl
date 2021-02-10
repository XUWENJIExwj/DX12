cbuffer cbSettings : register(b0)
{
    int   gBlurRadius;
    float w00;
    float w01;
    float w02;
    float w03;
    float w04;
    float w05;
    float w06;
    float w07;
    float w08;
    float w09;
    float w10;
    float w11;
    float w12;
    float w13;
    float w14;
    float w15;
    float w16;
    float w17;
    float w18;
    float w19;
    float w20;
    float w21;
    float w22;
    float w23;
    float w24;
    float w25;
    float w26;
    float w27;
    float w28;
    float w29;
    float w30;
    float w31;
    float w32;
    float w33;
    float w34;
    float w35;
    float w36;
    float w37;
    float w38;
    float w39;
    float w40;
}

static const int gMaxBlurRadius = 20;

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define NumThreads (256)
#define CacheSize  (NumThreads + 2 * gMaxBlurRadius)

groupshared float4 gCache[CacheSize];

[numthreads(NumThreads, 1, 1)]
void HorizontalBlurCS(int3 GroupThreadID : SV_GroupThreadID, int3 DispatchThreadID : SV_DispatchThreadID)
{
    float weights[41] =
    { 
        w00, w01, w02, w03, w04, w05, w06, w07, w08, w09,
        w10, w11, w12, w13, w14, w15, w16, w17, w18, w19,
        w20, w21, w22, w23, w24, w25, w26, w27, w28, w29,
        w30, w31, w32, w33, w34, w35, w36, w37, w38, w39,
        w40
    };

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
    float weights[41] =
    {
        w00, w01, w02, w03, w04, w05, w06, w07, w08, w09,
        w10, w11, w12, w13, w14, w15, w16, w17, w18, w19,
        w20, w21, w22, w23, w24, w25, w26, w27, w28, w29,
        w30, w31, w32, w33, w34, w35, w36, w37, w38, w39,
        w40
    };

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