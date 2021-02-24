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

#ifndef CASCADE_NUM
    #define CASCADE_NUM 3
#endif

// Include structures and functions for lighting.
#include "LightingUtil.hlsl"

struct MaterialData
{
	float4   DiffuseAlbedo;
    float4   CubeMapDiffuseAlbedo;
	float3   FresnelR0;
	float    Roughness;
	float4x4 MatTransform;
	uint     DiffuseMapIndex;
    uint     NormalMapIndex;
	uint     HeightMapIndex;
    int      BitangentSign;
    uint     UseACForPOM;
    uint     MaxSampleCount;
    uint     MinSampleCount;
    uint     CascadeDebugIndex;
    float    HeightScale;
    float    ShadowSoftening;
    float    floatPad0;
    float    floatPad1;
    bool     ShowSelfShadow;
    bool     boolPad0;
    bool     boolPad1;
    bool     boolPad2;
};

TextureCube gCubeMap : register(t0);
Texture2D gShadowMap[CASCADE_NUM] : register(t0, space1);

// An array of textures, which is only supported in shader model 5.1+.  Unlike Texture2DArray, the textures
// in this array can be different sizes and formats, making it more flexible than texture arrays.
// Textureの数によって、配列要素数を増やしていく
Texture2D gTextureMaps[41 + CASCADE_NUM] : register(t1);

// Put in space2, so the texture array does not overlap with these resources.  
// The texture array will occupy registers t0, t1, ..., t3 in space2. 
StructuredBuffer<MaterialData> gMaterialData : register(t0, space2);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
	float4x4 gTexTransform;
	uint gMaterialIndex;
	uint gObjPad0;
	uint gObjPad1;
	uint gObjPad2;
};

// Constant data that varies per material.
cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3   gEyePosWS;
    float    cbPerObjectPad1;
    float2   gRenderTargetSize;
    float2   gInvRenderTargetSize;
    float    gNearZ;
    float    gFarZ;
    float    gTotalTime;
    float    gDeltaTime;
    float4   gAmbientLight;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
    
    // CSM
    float4x4 gShadowView;
    float    gMaxBorderPadding;
    float    gMinBorderPadding;
    float    gCascadeBlendArea;
    float    cbPerObjectPad0;
    float4   gShadowBias;
    //float    gShadowBias[3];
    //float    cbPerObjectPad7;
    //float    gShadowBias;
    float4   gShadowTexScale[CASCADE_NUM];
    float4   gShadowTexOffset[CASCADE_NUM];
    int      gPCFBlurForLoopStart;
    int      gPCFBlurForLoopEnd;
    int      cbPerObjectPad2;
    int      cbPerObjectPad3;
    bool     gVisualCascade;
    bool     gBlendCascade;
    bool     cbPerObjectPad5;
    bool     cbPerObjectPad6;
    float4   gCascadeFrustumsEyeSpaceDepthsFloat;
    float4   gCascadeFrustumsEyeSpaceDepthsFloat4[CASCADE_NUM];
};

static const float4 gCascadeColorsMultiplier[8] =
{
    float4(1.5f, 0.0f, 0.0f, 1.0f),
	float4(0.0f, 1.5f, 0.0f, 1.0f),
	float4(0.0f, 0.0f, 5.5f, 1.0f),
	float4(1.5f, 0.0f, 5.5f, 1.0f),
	float4(1.5f, 1.5f, 0.0f, 1.0f),
	float4(1.0f, 1.0f, 1.0f, 1.0f),
	float4(0.0f, 1.0f, 5.5f, 1.0f),
	float4(0.5f, 3.5f, 0.75f, 1.0f)
};


float3x3 ComputeTBN(float3 NormalWS, float3 TangentWS, int BitangentSign)
{
    // Build orthonormal basis.
    float3 N = NormalWS;
    float3 T = normalize(TangentWS - dot(TangentWS, N) * N);
    // Textureによって、Bitangentの符号が異なって、反転する必要なことがある
    // 反転する必要があるTextureが多いので、先に-しておいて、
    // MaterialのBitangentSignと合わせて使う
    float3 B = -cross(N, T) * BitangentSign;
    
    return float3x3(T, B, N);
}

//---------------------------------------------------------------------------------------
// Transforms a normal map sample to world space.
//---------------------------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 NormalMapSample, float3 NormalWS, float3 TangentWS, int BitangentSign)
{
	// Uncompress each component from [0,1] to [-1,1].
    float3 normalTS = 2.0 * NormalMapSample - 1.0;

    float3 bumpedNormalWS = mul(normalTS, ComputeTBN(NormalWS, TangentWS, BitangentSign));

    return bumpedNormalWS;
}

//---------------------------------------------------------------------------------------
// PCF for shadow mapping.
//---------------------------------------------------------------------------------------
float CalcShadowFactor(float4 ShadowPosHS, float PCFBlurSize, int CascadeIndex)
{
    // Complete projection by doing division by w.
    ShadowPosHS.xyz /= ShadowPosHS.w;

    uint width, height, numMips;
    gShadowMap[CascadeIndex].GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0 / (float)width;
    float dy = 1.0 / (float)height;

    float percentLit = 0.0;
    
    for (int y = gPCFBlurForLoopStart; y < gPCFBlurForLoopEnd; ++y)
    {
        for (int x = gPCFBlurForLoopStart; x < gPCFBlurForLoopEnd; ++x)
        {
            // Depth in NDC space.
            float depth = ShadowPosHS.z;
            depth -= gShadowBias[CascadeIndex];
            percentLit += gShadowMap[CascadeIndex].SampleCmpLevelZero(gsamShadow, float2(ShadowPosHS.x + x * dx, ShadowPosHS.y + y * dy), depth).r;
        }
    }
    
    return percentLit / PCFBlurSize;
}

float4 ComputeShadowTexCoord(float4 ShadowPosLiS, int CasecadeIndex)
{
    float4 texC = ShadowPosLiS * gShadowTexScale[CasecadeIndex];
    texC += gShadowTexOffset[CasecadeIndex];
    return texC;
}

void ComputeCascadeIndex(float4 ShadowPosLiS, out float4 ShadowMapTexHS, out int CascadeIndex)
{
    CascadeIndex = 0;
    ShadowMapTexHS = 0.0;
    
    int cascadeFound = 0;
    
    [unroll]
    for (int i = 0; i < CASCADE_NUM && cascadeFound == 0; ++i)
    {
        ShadowMapTexHS = ComputeShadowTexCoord(ShadowPosLiS, i);
 
        if (min(ShadowMapTexHS.x, ShadowMapTexHS.y) > gMinBorderPadding &&
            max(ShadowMapTexHS.x, ShadowMapTexHS.y) < gMaxBorderPadding)//&& ShadowMapTexHS.z > 0.0 && gMaxBorderPadding && ShadowMapTexHS.z < 1.0)
        {
            CascadeIndex = i;
            cascadeFound = 1;
        }
    }
}

//--------------------------------------------------------------------------------------
// Calculate amount to blend between two cascades and the band where blending will occure.
//--------------------------------------------------------------------------------------
void CalculateBlendAmountForInterval(int CurrentCascadeIndex, in out float PixelDepth, in out float CurrentPixelsBlendBandLocation, out float BlendBetweenCascadesAmount)
{
    // We need to calculate the band of the current shadow map where it will fade into the next cascade.
    // We can then early out of the expensive PCF for loop.
    // 
    float blendInterval = gCascadeFrustumsEyeSpaceDepthsFloat4[CurrentCascadeIndex].x;
    //if( iNextCascadeIndex > 1 ) 
    int blendIntervalbelowIndex = min(0, CurrentCascadeIndex - 1);
    PixelDepth -= gCascadeFrustumsEyeSpaceDepthsFloat4[blendIntervalbelowIndex].x;
    blendInterval -= gCascadeFrustumsEyeSpaceDepthsFloat4[blendIntervalbelowIndex].x;
    
    // The current pixel's blend band location will be used to determine when we need to blend and by how much.
    CurrentPixelsBlendBandLocation = PixelDepth / blendInterval;
    CurrentPixelsBlendBandLocation = 1.0f - CurrentPixelsBlendBandLocation;
    // The fBlendBetweenCascadesAmount is our location in the blend band.
    BlendBetweenCascadesAmount = CurrentPixelsBlendBandLocation / gCascadeBlendArea;
}

void ComputeShadowFactorAndCurrentCascadeIndexWithCSM(float DepthCamS, float4 ShadowPosLiS, in out float ShadowFactor, in out int CurrentCascadeIndex)
{
    float pcfBlursize = gPCFBlurForLoopEnd - gPCFBlurForLoopStart;
    pcfBlursize *= pcfBlursize;
    float4 shadowMapTexHS = 0.0;
    float currentPixelDepth1f = DepthCamS;
    float4 shadowPosLiS = ShadowPosLiS;
    
    // ForInterval
    float4 currentPixelDepth = currentPixelDepth1f;
    float4 comparison = (currentPixelDepth > gCascadeFrustumsEyeSpaceDepthsFloat);
    float index = dot(float4(CASCADE_NUM > 0, CASCADE_NUM > 1, CASCADE_NUM > 2, CASCADE_NUM > 3), comparison);
    index = min(index, CASCADE_NUM - 1);
    CurrentCascadeIndex = (int)index;
    shadowMapTexHS = ComputeShadowTexCoord(shadowPosLiS, CurrentCascadeIndex);
    float shadowFactor_blend0 = CalcShadowFactor(shadowMapTexHS, pcfBlursize, CurrentCascadeIndex);
    
    //Blend
    //if (gBlendCascade)
    //{
    //    int nextCascadeIndex = 0;
    //    nextCascadeIndex = min(CASCADE_NUM - 1, CurrentCascadeIndex + 1);
    //    float blendBetweenCascadesAmount = 1.0;
    //    float currentPixelsBlendBandLocation = 1.0;
    //    CalculateBlendAmountForInterval(CurrentCascadeIndex, currentPixelDepth1f, currentPixelsBlendBandLocation, blendBetweenCascadesAmount);
    //    float shadowFactor_blend1 = 1.0;
    //    float4 shadowMapTexHS_blend = 0.0;
    //    if (currentPixelsBlendBandLocation < gCascadeBlendArea)
    //    {
    //        shadowMapTexHS_blend = ComputeShadowTexCoord(shadowPosLiS, nextCascadeIndex);
        
    //        if (currentPixelsBlendBandLocation < gCascadeBlendArea)
    //        {
    //            shadowFactor_blend1 = CalcShadowFactor(shadowMapTexHS_blend, pcfBlursize, nextCascadeIndex);
    //            shadowFactor_blend0 = lerp(shadowFactor_blend1, shadowFactor_blend0, blendBetweenCascadesAmount);
    //        }
    //    }
    //}
    ShadowFactor *= shadowFactor_blend0;
}