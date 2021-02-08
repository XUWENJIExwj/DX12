cbuffer cbSettings : register(b0)
{
    int2 gScreenSize;
    int2 gCenter;
    int  gSampleDistance;
    int  gSampleStrength;
    int  cbPad0;
    int  cbPad1;
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
void RadialBlurCS(int3 DispatchThreadID : SV_DispatchThreadID)
{
    int2 direction = gCenter - DispatchThreadID.xy;
    float distance = length(float2(direction) / float2(gScreenSize)); // Normalize���Ă���
    
    float4 color = gInput[DispatchThreadID.xy];
    float4 sum = color;
    for (int i = 0; i < 10; ++i)
    {
        sum += gInput[DispatchThreadID.xy + direction * samples[i] / gSampleDistance];
    }
    sum /= 11.0;
    float t = saturate(distance * gSampleStrength);
    gOutput[DispatchThreadID.xy] = lerp(color, sum, t);
}