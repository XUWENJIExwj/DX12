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
	float4 PosHS        : SV_POSITION;
    float3 PosWS        : POSITION;
    float4 ShadowPosLiS : POSITION1;
    float3 NormalWS     : NORMAL;
    float3 TangentWS    : TANGENT;
	float2 TexC         : TEXCOORD;
    float  DepthCamS    : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	// Fetch the material data.
	MaterialData matData = gMaterialData[gMaterialIndex];
	
    // Transform to world space.
    float4 posWS = mul(float4(vin.PosLS, 1.0f), gWorld);
    vout.PosWS = posWS.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalWS = mul(vin.NormalLS, (float3x3) gWorld);
    
    vout.TangentWS = mul(vin.TangentLS, (float3x3) gWorld);

    // Transform to homogeneous clip space.
    vout.PosHS = mul(posWS, gViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.TexC, 0.0, 1.0), gTexTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;
    
    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosLiS = mul(posWS, gShadowView);
    
    vout.DepthCamS = mul(posWS, gView).z;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	// Fetch the material data.
	MaterialData matData = gMaterialData[gMaterialIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo;
    float4 cubeMapDiffuseAlbedo = matData.CubeMapDiffuseAlbedo;
	float3 fresnelR0 = matData.FresnelR0;
	float roughness = matData.Roughness;
	uint diffuseTexIndex = matData.DiffuseMapIndex;
    uint normalMapIndex = matData.NormalMapIndex;
    int bitangentSign = matData.BitangentSign;
    
    // Interpolating normal can unnormalize it, so renormalize it.
    float3 normalWS = normalize(pin.NormalWS);
    
    float4 normalMapSample = gTextureMaps[normalMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    float3 bumpedNormalWS = NormalSampleToWorldSpace(normalMapSample.rgb, normalWS, pin.TangentWS, bitangentSign);
    
    // Uncomment to turn off normal mapping.
	//bumpedNormalWS = pin.NormalWS;

	// Dynamically look up the texture in the array.
    diffuseAlbedo *= gTextureMaps[diffuseTexIndex].Sample(gsamAnisotropicWrap, pin.TexC) * cubeMapDiffuseAlbedo;

    // Vector from point being lit to eye. 
    float3 toEyeWS = normalize(gEyePosWS - pin.PosWS);

    // Light terms.
    float4 ambient = gAmbientLight * diffuseAlbedo;

    const float shininess = max(1.0 - roughness, 0.01);
    Material mat = { diffuseAlbedo, fresnelR0, shininess };

    // CSM
    float3 shadowFactor = float3(1.0, 1.0, 1.0);
    int currentCascadeIndex = 0;
    ComputeShadowFactorAndCurrentCascadeIndexWithCSM(pin.DepthCamS, pin.ShadowPosLiS, shadowFactor[0], currentCascadeIndex);
    float4 visualCascadeColor = 1.0f;
    if (gVisualCascade)
    {
        visualCascadeColor = gCascadeColorsMultiplier[currentCascadeIndex];
    }
    
    float4 directLight = ComputeLighting(gLights, mat, pin.PosWS,
        bumpedNormalWS, toEyeWS, shadowFactor);

    float4 litColor = ambient + directLight;

	// Add in specular reflections.
    float3 r = reflect(-toEyeWS, bumpedNormalWS);
    float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r) * cubeMapDiffuseAlbedo;
    float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalWS, r);
	litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
    
    litColor *= visualCascadeColor;

    // Common convention to take alpha from diffuse albedo.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}


