Texture2D gInputA : register(t0);
Texture2D gInputB : register(t1);
RWTexture2D<float4> gOutput : register(u0);

[numthreads(16, 16, 1)]
void BlendOpAddCS(int3 DispatchThreadID : SV_DispatchThreadID)
{
    gOutput[DispatchThreadID.xy] = gInputA[DispatchThreadID.xy] + gInputB[DispatchThreadID.xy];
}