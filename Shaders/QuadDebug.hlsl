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
    
    MaterialData matData = gMaterialData[gMaterialIndex];

    // For 2DPolygon, gWorld has been transformed to WVP
    vout.PosHS = mul(float4(vin.PosLS, 1.0), gWorld);
    float4 texC = mul(float4(vin.TexC, 0.0, 1.0), gTexTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    MaterialData matData = gMaterialData[gMaterialIndex];
    float4 color = 1.0;
    
#ifdef SHADOW
    color.rgb = gTextureMaps[matData.DiffuseMapIndex].Sample(gsamLinearWrap, pin.TexC).rrr;
#endif
    
#ifdef QUAD
    color.rgb = gTextureMaps[matData.DiffuseMapIndex].Sample(gsamLinearWrap, pin.TexC).rgb;
#endif
    return color;
}


