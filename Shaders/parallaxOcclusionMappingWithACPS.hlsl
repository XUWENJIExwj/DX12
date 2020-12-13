#include "mathHelper.hlsl"
#include "lightHelper.hlsl"

Texture2D g_Texture : register(t0);
Texture2D g_TextureNormal : register(t1);
Texture2D g_TextureHeight : register(t2);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    // 仮
	int gMaxSampleCount = 1024;
	int gMinSampleCount = 8;
	bool gUseTexure = true;
	bool gAlphaClip = false;
	bool gDisplayShadows = true;
    
    // Normalize the Light
	float3 lightDirWS = normalize(Light.Direction.xyz);
    
    // The toEyeWS vector is used in lighting.
	float3 toEyeWS = normalize(CameraPosition.xyz - In.PositionWS.xyz);

    // Parallax Occlusion calculations to find the texture coords to use.
	float3 viewDirWS = -toEyeWS;
    
    // Build orthonormal basis.
	float3 N = normalize(In.NormalWS);
	float3 T = normalize(In.TangentWS - dot(In.TangentWS, N) * N);
	float3 B = cross(N, T);
	float3x3 toTangent = transpose(float3x3(T, B, N));
	float3 viewDirTS = mul(viewDirWS, toTangent);
	float2 maxParallaxOffset = -viewDirTS.xy * Parameter.x / viewDirTS.z;
    
    // Vary number of samples based on view angle between the eye and
    // the surface normal. (Head-on angles require less samples than
    // glancing angles.)
	int sampleCount = (int) lerp(gMaxSampleCount, gMinSampleCount, dot(toEyeWS, N));
	float zStep = 1.0f / (float) sampleCount;
    
	float2 texStep = maxParallaxOffset * zStep;
    
    // Precompute texture gradients since we cannot compute texture
    // gradients in a loop. Texture gradients are used to select the right
    // mipmap level when sampling textures. Then we use Texture2D.SampleGrad()
    // instead of Texture2D.Sample().
	float2 dx = ddx(In.TexCoord);
	float2 dy = ddy(In.TexCoord);
    
	int sampleIndex = 0;
	float2 currTexOffset = 0;
	float2 prevTexOffset = 0;
	float2 finalTexOffset = 0;
	float currRayZ = 1.0f - zStep;
	float prevRayZ = 1.0f;
	float currHeight = 0.0f;
	float prevHeight = 0.0f;
    
    // Ray trace the heightfield.
	while (sampleIndex < sampleCount + 1)
	{
		currHeight = g_TextureHeight.SampleGrad(g_SamplerState, In.TexCoord + currTexOffset, dx, dy).a;
        
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
    
    // SoftShadowのためのNew法線（texCoordの偏移よる法線の偏移後の向きが見た目に合わない）
    float3 currPointTS = float3(currTexOffset, currHeight);
    float3 prevPointTS = float3(prevTexOffset, prevHeight);
    float3 lightDirTS = -mul(lightDirWS, toTangent);
    float3 currPointYTS = normalize(currPointTS - prevPointTS);
    currPointYTS = abs(dot(lightDirTS, normalize(currPointTS - prevPointTS))) * currPointYTS;
    float3 currPrevPointNTS = -(lightDirTS - currPointYTS);
    
    // Use these texture coordinates for subsequent texture
    // fetches (color map, normal map, etc.).
	float2 parallaxTex = In.TexCoord + finalTexOffset;
    
    // SoftShadow
	float shadowFactor = 1.0;
    
    gDisplayShadows = true;
    
    // Normal mapping
    float3 normalMapSample = g_TextureNormal.Sample(g_SamplerState, parallaxTex).rgb;
    float3 normalWS = NormalSampleToWorldSpace(float3(normalMapSample.x, normalMapSample.y, normalMapSample.z), N, T);
    
    //if (gDisplayShadows)
    if (gDisplayShadows && dot(lightDirTS, currPrevPointNTS) < 0)
    //if (gDisplayShadows && dot(lightDirWS, normalWS) < 0)
    {
        sampleCount = (int) lerp(gMaxSampleCount, gMinSampleCount, dot(-lightDirWS, N));
        
        maxParallaxOffset = float2(lightDirTS.x, lightDirTS.y) * Parameter.x / lightDirTS.z;
        
        prevRayZ = g_TextureHeight.Sample(g_SamplerState, parallaxTex).a;
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
            currHeight = g_TextureHeight.SampleGrad(g_SamplerState, parallaxTex + currTexOffset, dx, dy).a;
            
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
            shadowFactor = 1 - saturate((1 - (g_TextureHeight.Sample(g_SamplerState, parallaxTex + finalTexOffset).a - g_TextureHeight.Sample(g_SamplerState, parallaxTex).a)) * Parameter.y);
        }
    }
    
    // Texturing
    // Default to multiplicative identity.
	float4 texColor = float4(1, 1, 1, 1);
    if (gUseTexure)
	{
        // Sample texture.
		texColor = g_Texture.Sample(g_SamplerState, parallaxTex);
		if (gAlphaClip)
		{
            // Discard pixel if texture alpha < 0.1. Note that we do this
            // test as soon as possible so that we can potentially exit
            // the shader early, thereby skipping the rest of the shader
            // code.
			clip(texColor.a - 0.1f);
		}
	}
    
    // Lighting. 
	outDiffuse = ComputeDirectionalLight(texColor, In.Diffuse, lightDirWS, normalWS, toEyeWS, Parameter.z);
	outDiffuse.rgb *= shadowFactor;
}