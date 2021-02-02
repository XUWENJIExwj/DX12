cbuffer cbSettings : register(b0)
{
    int gScreenWidth;
    int gScrennHeight;
    int gCenterX;
    int gCenterY;
    int gSampleDistance;
    int gSampleStrength;
    int cbPad0;
    int cbPad1;
}

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

static const int samples[10] =
{
    -8,
    -5,
    -3,
    -2,
    -1,
     1,
     2,
     3,
     5,
     8
};

[numthreads(16, 16, 1)]
void RadialBlurCS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int2 center = int2(gCenterX, gCenterY);
    int2 direction = center - dispatchThreadID.xy;
    int2 screenSize = int2(gScreenWidth, gScrennHeight);
    float distance = length(float2(direction) / float2(screenSize)); // Normalize
    
    float4 color = gInput[dispatchThreadID.xy];
    float4 sum = color;
    for (int i = 0; i < 10; ++i)
    {
        sum += gInput[dispatchThreadID.xy + direction * samples[i] / gSampleDistance];
    }
    sum /= 11.0;
    float t = saturate(distance * gSampleStrength);
    gOutput[dispatchThreadID.xy] = lerp(color, sum, t);
}