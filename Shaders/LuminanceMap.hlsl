cbuffer cbSettings : register(b0)
{
    float LuminanceThreshold;
    float LuminanceStrength;
}

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

float ComputeLuminance(float3 color)
{
    return dot(color, float3(0.299, 0.587, 0.114));
}

[numthreads(16, 16, 1)]
void LuminanceMapCS(int3 DispatchThreadID : SV_DispatchThreadID)
{
    float4 color = gInput[DispatchThreadID.xy];
    color.a = 0.0;
    float luminance = ComputeLuminance(color.rgb);
    gOutput[DispatchThreadID.xy] = 0.0;
    if (luminance > LuminanceThreshold)
    {
        gOutput[DispatchThreadID.xy] = color * LuminanceStrength;
    }
}