#include "Common.hlsl"

struct VertexIn
{
	float3 PosLS    : POSITION;
	float3 NormalLS : NORMAL;
	float2 TexC     : TEXCOORD;
};

struct VertexOut
{
	float4 PosHS : SV_POSITION;
    float3 PosLS : POSITION;
};
 
VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Use local vertex position as cubemap lookup vector.
	vout.PosLS = vin.PosLS;
	
	// Transform to world space.
	float4 posWS = mul(float4(vin.PosLS, 1.0f), gWorld);

	// Always center sky about camera.
	posWS.xyz += gEyePosW;

	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	vout.PosHS = mul(posWS, gViewProj).xyww;
	
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return gCubeMap.Sample(gsamLinearWrap, pin.PosLS);
}

