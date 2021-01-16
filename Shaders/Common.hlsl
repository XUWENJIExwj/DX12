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
    int      IntPad0;
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
Texture2D gShadowMap : register(t0, space1);

// An array of textures, which is only supported in shader model 5.1+.  Unlike Texture2DArray, the textures
// in this array can be different sizes and formats, making it more flexible than texture arrays.
// Textureの数によって、配列要素数を増やしていく
Texture2D gTextureMaps[36] : register(t1);

// Put in space1, so the texture array does not overlap with these resources.  
// The texture array will occupy registers t1, ..., t3 in space0. 
StructuredBuffer<MaterialData> gMaterialData : register(t1, space1);

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
    float4x4 gShadowTransform;
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
    float3 normalTS = 2.0f * NormalMapSample - 1.0f;

    float3 bumpedNormalWS = mul(normalTS, ComputeTBN(NormalWS, TangentWS, BitangentSign));

    return bumpedNormalWS;
}

//---------------------------------------------------------------------------------------
// PCF for shadow mapping.
//---------------------------------------------------------------------------------------
float CalcShadowFactor(float4 ShadowPosHS)
{
    // Complete projection by doing division by w.
    ShadowPosHS.xyz /= ShadowPosHS.w;

    // Depth in NDC space.
    float depth = ShadowPosHS.z;

    uint width, height, numMips;
    gShadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float) width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx),  float2(0.0f, -dx),  float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx),  float2(0.0f, +dx),  float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
            ShadowPosHS.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}