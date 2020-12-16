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
    
    vout.TangentWS = mul(vin.TangentLS, (float3x3) gWorld);

    // Transform to homogeneous clip space.
    vout.PosHS = mul(posWS, gViewProj);
	
	// Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;
	
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
    
    MaterialOfHeightData matExData = gMaterialOfHeightData[gMaterialIndex];
    float heightScale = matExData.HeightScale;
    int maxSampleCount = matExData.MaxSampleCount;
    int minSampleCount = matExData.MinSampleCount;
    int useACForPOM = matExData.UseACForPOM;
    
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
    float zStep = 1.0f / (float)sampleCount;
    
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
    float currRayZ = 1.0f - zStep;
    float prevRayZ = 1.0f;
    float4 height = 0.0f;
    float currHeight = 0.0f;
    float prevHeight = 0.0f;
    
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

    const float shininess = max(1.0f - roughness, 0.01);
    Material mat = { diffuseAlbedo, fresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosWS,
        bumpedNormalWS, toEyeWS, shadowFactor);

    float4 litColor = ambient + directLight;

	// Add in specular reflections.
    float3 r = reflect(-toEyeWS, bumpedNormalWS);
    float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r) * cubeMapDiffuseAlbedo;
    float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalWS, r);
    litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;

    // Common convention to take alpha from diffuse albedo.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}

float3 ComputeTexCoordFromHeightMap(float3 ViewDirTS)
{
    
    return float3(0, 0, 0);
}