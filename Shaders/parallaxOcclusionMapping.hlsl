// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 1
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
    //int cascadeIndex = 0;
    
    VertexOut vout = (VertexOut) 0.0;

	// Fetch the material data.
    MaterialData matData = gMaterialData[gMaterialIndex];
	
    // Transform to world space.
    float4 posWS = mul(float4(vin.PosLS, 1.0), gWorld);
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
    uint heightMapIndex = matData.HeightMapIndex;
    int bitangentSign = matData.BitangentSign;
    int maxSampleCount = matData.MaxSampleCount;
    int minSampleCount = matData.MinSampleCount;
    int useACForPOM = matData.UseACForPOM;
    float heightScale = matData.HeightScale;
    float shadowSoftening = matData.ShadowSoftening;
    bool showSelfShadow = matData.ShowSelfShadow;
    
    // Interpolating normal can unnormalize it, so renormalize it.
    float3 normalWS = normalize(pin.NormalWS);
    
    // The toEyeWS vector is used in lighting.
    float3 toEyeWS = normalize(gEyePosWS - pin.PosWS);
    
    // Parallax Occlusion calculations to find the texture coords to use.
    float3 viewDirWS = -toEyeWS;
    
    // Build orthonormal basis.
    float3x3 toTangent = transpose(ComputeTBN(normalWS, pin.TangentWS, -bitangentSign));
    float3 viewDirTS = mul(viewDirWS, toTangent);
    float2 maxParallaxOffset = -viewDirTS.xy * heightScale / viewDirTS.z;
    // Vary number of samples based on view angle between the eye and
    // the surface normal. (Head-on angles require less samples than
    // glancing angles.)
    int sampleCount = (int)lerp(maxSampleCount, minSampleCount, dot(toEyeWS, normalWS));
    float zStep = 1.0 / (float)sampleCount;
    
    float2 texStep = maxParallaxOffset * zStep;
    
     // Precompute texture gradients since we cannot compute texture
    // gradients in a loop. Texture gradients are used to select the right
    // mipmap level when sampling textures. Then we use Texture2D.SampleGrad()
    // instead of Texture2D.Sample().
    float2 dx = ddx(pin.TexC);
    float2 dy = ddy(pin.TexC);
    
    int sampleIndex = 0;
    float2 currTexOffset = 0;
    float2 prevTexOffset = 0;
    float2 finalTexOffset = 0;
    float currRayZ = 1.0 - zStep;
    float prevRayZ = 1.0;
    float4 height = 0.0;
    float currHeight = 0.0;
    float prevHeight = 0.0;
    
    // Ray trace the heightfield.
    while (sampleIndex < sampleCount + 1)
    {
        height = gTextureMaps[heightMapIndex].SampleGrad(gsamAnisotropicWrap, pin.TexC + currTexOffset, dx, dy);
        currHeight = useACForPOM * height.a + (1 - useACForPOM) * height.r;
        
        // Did we cross the height profile?
        if (currHeight > currRayZ)
        {
            // Do ray/line segment intersection and compute final tex offset.
            float t = (prevHeight - prevRayZ) / (prevHeight - currHeight + currRayZ - prevRayZ);
            finalTexOffset = prevTexOffset + t * texStep;
            
            // Exit loop.
            sampleIndex = sampleCount + 1;
        }
        else
        {
            ++sampleIndex;
            
            prevTexOffset = currTexOffset;
            
            prevRayZ = currRayZ;
            prevHeight = currHeight;
            
            currTexOffset += texStep;

            // Negative because we are going "deeper" into the surface.
            currRayZ -= zStep;
        }
    }
    
    // Use these texture coordinates for subsequent texture
    // fetches (color map, normal map, etc.).
    float2 parallaxTex = pin.TexC + finalTexOffset;
    
    // Normal mapping
    float4 normalMapSample = gTextureMaps[normalMapIndex].Sample(gsamAnisotropicWrap, parallaxTex);
    float3 bumpedNormalWS = NormalSampleToWorldSpace(normalMapSample.rgb, normalWS, pin.TangentWS, bitangentSign);
    
    // Uncomment to turn off normal mapping.
	//bumpedNormalWS = pin.NormalWS;

	// Dynamically look up the texture in the array.
    diffuseAlbedo *= gTextureMaps[diffuseTexIndex].Sample(gsamAnisotropicWrap, parallaxTex) * cubeMapDiffuseAlbedo;

    // Light terms.
    float4 ambient = gAmbientLight * diffuseAlbedo;

    const float shininess = max(1.0 - roughness, 0.01);
    Material mat = { diffuseAlbedo, fresnelR0, shininess };

    // SelfShadow
    float3 shadowFactor = 1.0;
    if(showSelfShadow)
    {
        // 最初の平行光源だけでSelfShadowを計算（処理コストを下げるため）
        float3 lightDirWS = normalize(-gLights[0].Direction);
        float3 lightDirTS = mul(lightDirWS, toTangent);
        
        sampleCount = (int)lerp(maxSampleCount, minSampleCount, dot(lightDirWS, normalWS));
        maxParallaxOffset = float2(lightDirTS.x, lightDirTS.y) * heightScale / lightDirTS.z;
        
        float4 rayZ = gTextureMaps[heightMapIndex].Sample(gsamAnisotropicWrap, parallaxTex);
        prevRayZ = useACForPOM * rayZ.a + (1 - useACForPOM) * rayZ.r;
        prevHeight = prevRayZ;
        
        zStep = (1.0 - prevRayZ) / (float) sampleCount;
        
        currRayZ = prevRayZ + zStep;
        
        texStep = maxParallaxOffset * zStep;
        
        dx = ddx(parallaxTex);
        dy = ddy(parallaxTex);
        
        sampleIndex = 0;
        currTexOffset = texStep;
        prevTexOffset = 0;
        
        bool checkShadow = false;
        
        while (sampleIndex < sampleCount + 1)
        {
            currHeight = gTextureMaps[heightMapIndex].SampleGrad(gsamAnisotropicWrap, parallaxTex + currTexOffset, dx, dy).r;
            
            if (currHeight > currRayZ)
            {
                sampleIndex = sampleCount + 1;
                checkShadow = true;
                float t = (prevHeight - prevRayZ) / (prevHeight - currHeight + currRayZ - prevRayZ);
                finalTexOffset = prevTexOffset + t * texStep;
            }
            else
            {
                ++sampleIndex;
                
                prevTexOffset = currTexOffset;
                prevHeight = currHeight;
                prevRayZ = currRayZ;
                currTexOffset += texStep;
                currRayZ += zStep;
            }
        }
        
        if (checkShadow)
        {
            float4 shadow = gTextureMaps[heightMapIndex].Sample(gsamAnisotropicWrap, parallaxTex + finalTexOffset) - gTextureMaps[heightMapIndex].Sample(gsamAnisotropicWrap, parallaxTex);
            float finalShadow = useACForPOM * shadow.a + (1 - useACForPOM) * shadow.r;
            //shadowFactor[0] = 1 - saturate((1 - finalShadow) * shadowSoftening);
            shadowFactor[0] = useACForPOM * saturate((1 - finalShadow) * (1 - shadowSoftening * 0.5)) + (1 - useACForPOM) * (1 - saturate((1 - finalShadow) * shadowSoftening));
        }
    }
    
    // Cascade
    float pcfBlursize = gPCFBlurForLoopEnd - gPCFBlurForLoopStart;
    pcfBlursize *= pcfBlursize;
    float4 shadowMapTexHS = 0.0;
    int currentCascadeIndex = 0;
    float currentPixelDepth1f = pin.DepthCamS;
    float4 shadowPosLiS = pin.ShadowPosLiS;
    
    // ForInterval
    float4 currentPixelDepth = currentPixelDepth1f;
    float4 comparison = (currentPixelDepth > gCascadeFrustumsEyeSpaceDepthsFloat);
    float index = dot(float4(CASCADE_NUM > 0, CASCADE_NUM > 1, CASCADE_NUM > 2, CASCADE_NUM > 3), comparison);
    index = min(index, CASCADE_NUM - 1);
    currentCascadeIndex = (int)index;
    shadowMapTexHS = ComputeShadowTexCoord(shadowPosLiS, currentCascadeIndex);
    float shadowFactor_blend0 = CalcShadowFactor(shadowMapTexHS, pcfBlursize, currentCascadeIndex);
    
    ////Blend
    //if (gBlendCascade)
    //{
    //    int nextCascadeIndex = 1;
    //    nextCascadeIndex = min(CASCADE_NUM - 1, currentCascadeIndex + 1);
    //    float blendBetweenCascadesAmount = 1.0;
    //    float currentPixelsBlendBandLocation = 1.0;
    //    CalculateBlendAmountForInterval(currentCascadeIndex, currentPixelDepth1f, currentPixelsBlendBandLocation, blendBetweenCascadesAmount);
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
    shadowFactor[0] *= shadowFactor_blend0;
    
    // ForMap
    //ComputeCascadeIndex(shadowPosLiS, shadowMapTexHS, currentCascadeIndex);
    //shadowFactor[0] *= CalcShadowFactor(shadowMapTexHS, pcfBlursize, currentCascadeIndex);
    
    //if (gBlendCascade)
    //{
    //    // Blend Between Cascade Layers
    //    int nextCascadeIndex = 1;
    //    nextCascadeIndex = min(CASCADE_NUM - 1, currentCascadeIndex + 1);
    //    float blendBetweenCascadesAmount = 1.0;
    //    float currentPixelsBlendBandLocation = 1.0;
    //    CalculateBlendAmountForMap(shadowMapTexHS, currentPixelsBlendBandLocation, blendBetweenCascadesAmount);
        
    //    float shadowFactor_blend = 1.0;
    //    float4 shadowMapTexHS_blend = 0.0;
    //    if (currentPixelsBlendBandLocation < gCascadeBlendArea)
    //    {
    //        shadowMapTexHS_blend = ComputeShadowTexCoord(shadowPosLiS, nextCascadeIndex);
        
    //        if (currentPixelsBlendBandLocation < gCascadeBlendArea)
    //        {
    //            shadowFactor_blend = CalcShadowFactor(shadowMapTexHS_blend, pcfBlursize, nextCascadeIndex);
    //            shadowFactor[0] = lerp(shadowFactor_blend, shadowFactor[0], blendBetweenCascadesAmount);
    //        }
    //    }
    //}
    
    float4 directLight = ComputeLighting(gLights, mat, pin.PosWS,
        bumpedNormalWS, toEyeWS, shadowFactor);

    float4 litColor = ambient + directLight;

	// Add in specular reflections.
    float3 r = reflect(-toEyeWS, bumpedNormalWS);
    float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r) * cubeMapDiffuseAlbedo;
    float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalWS, r);
    litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
    
    // CascadeVisualOn
    float4 visualCascadeColor = 1.0f;
    if(gVisualCascade)
    {
        visualCascadeColor = gCascadeColorsMultiplier[currentCascadeIndex];
    }
    litColor *= visualCascadeColor;

    // Common convention to take alpha from diffuse albedo.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}