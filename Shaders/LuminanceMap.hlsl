cbuffer cbSettings : register(b0)
{
    float gBaseColor;
    float gLuminanceThreshold;
    float gLuminanceStrength;
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
    float4 baseColor = gBaseColor;
    baseColor.a = 0.0;
    gOutput[DispatchThreadID.xy] = baseColor;
    if (luminance > gLuminanceThreshold)
    {
        gOutput[DispatchThreadID.xy] = color * gLuminanceStrength;
    }
}