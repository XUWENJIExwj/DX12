#include "Common.hlsl"

struct VertexIn
{
	float3 PosLS : POSITION;
	float2 TexC  : TEXCOORD;
};

struct VertexOut
{
	float4 PosHS : SV_POSITION;
	float2 TexC  : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

    // Already in homogeneous clip space.
    vout.PosHS = float4(vin.PosLS, 1.0f);
	
	vout.TexC = vin.TexC;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return float4(gShadowMap.Sample(gsamLinearWrap, pin.TexC).rrr, 1.0f);
}


