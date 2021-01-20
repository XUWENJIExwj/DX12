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
	float4 posWS = mul(float4(vin.PosLS, 1.0), gWorld);

	// Always center sky about camera.
	posWS.xyz += gEyePosWS;

	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	vout.PosHS = mul(posWS, gViewProj).xyww;
	
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    MaterialData matData = gMaterialData[gMaterialIndex];
    float4 diffuseAlbedo = gCubeMap.Sample(gsamLinearWrap, pin.PosLS);
    float4 litColor = diffuseAlbedo * matData.DiffuseAlbedo;
    litColor.a = diffuseAlbedo.a;
	return litColor;
}

