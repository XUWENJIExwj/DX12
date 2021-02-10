cbuffer cbSettings : register(b0)
{
    int2 gScreenSize;
    int2 gCenter;
    int  gSampleDistance;
    int  gSampleStrength;
}

Texture2D gInput : register(t0);
Texture2D gNone : register(t1);
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
void RadialBlurCS(int3 DispatchThreadID : SV_DispatchThreadID)
{
    int2 direction = gCenter - DispatchThreadID.xy;
    
    float4 color = gInput[DispatchThreadID.xy];
    float4 sum = color;
    for (int i = 0; i < 10; ++i)
    {
        sum += gInput[DispatchThreadID.xy + direction * samples[i] / gSampleDistance];
    }
    sum /= 11.0;
    
    float distance = length(float2(direction) / float2(gScreenSize)); // Normalize‚µ‚Ä‚©‚ç
    float t = saturate(distance * gSampleStrength);
    gOutput[DispatchThreadID.xy] = lerp(color, sum, t);
}