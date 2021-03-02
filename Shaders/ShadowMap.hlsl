#include "Common.hlsl"

struct VertexIn
{
	float3 PosLS   : POSITION;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosHS   : SV_POSITION;
	float2 TexC    : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0;

	MaterialData matData = gMaterialData[gMaterialIndex];
	
    // Transform to world space.
    float4 posWS = mul(float4(vin.PosLS, 1.0), gWorld);

    // Transform to homogeneous clip space.
    // gViewProj is based on light
    vout.PosHS = mul(posWS, gViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.TexC, 0.0, 1.0), gTexTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;
	
    return vout;
}

// This is only used for alpha cut out geometry, so that shadows 
// show up correctly.  Geometry that does not need to sample a
// texture can use a NULL pixel shader for depth pass.
void PS(VertexOut pin) 
{
	// Fetch the material data.
	MaterialData matData = gMaterialData[gMaterialIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo;
    uint diffuseMapIndex = matData.DiffuseMapIndex;
	
	// Dynamically look up the texture in the array.
	diffuseAlbedo *= gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    
#ifdef ALPHA_TEST
    // Discard pixel if texture alpha < 0.1.  We do this test as soon 
    // as possible in the shader so that we can potentially exit the
    // shader early, thereby skipping the rest of the shader code.
    clip(diffuseAlbedo.a - 0.1);
#endif
}


