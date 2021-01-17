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
    VertexOut vout = (VertexOut) 0.0;

    // For 2DPolygon, gWorld has been transformed to WVP
    vout.PosHS = mul(float4(vin.PosLS, 1.0), gWorld);
    vout.TexC = vin.TexC;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return float4(gShadowMap.Sample(gsamLinearWrap, pin.TexC).rrr, 1.0);
}


