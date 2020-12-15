// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

#include "Common.hlsl"

struct VertexIn
{
    float3 PosLS     : POSITION;
    float3 NormalLS  : NORMAL;
    float3 TangentLS : TANGENT;
    float2 TexC      : TEXCOORD;
};

struct VertexOut
{
    float4 PosHS     : SV_POSITION;
    float3 PosWS     : POSITION;
    float3 NormalWS  : NORMAL;
    float3 TangentWS : TANGENT;
    float2 TexC      : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;

	// Fetch the material data.
    MaterialData matData = gMaterialData[gMaterialIndex];
	
    // Transform to world space.
    float4 posWS = mul(float4(vin.PosLS, 1.0f), gWorld);
    vout.PosWS = posWS.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalWS = mul(vin.NormalLS, (float3x3) gWorld);
    
    vout.TangentWS = mul(vin.TangentLS, (float3x3) gWorld) * matData.TangentSign;

    // Transform to homogeneous clip space.
    vout.PosHS = mul(posWS, gViewProj);
	
	// Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    
    
    float4 litColor;
    return litColor;
}