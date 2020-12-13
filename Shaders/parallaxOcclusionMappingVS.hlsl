

//#include "mathHelper.hlsl"

//void main(in VS_IN In, out PS_IN Out)
//{
//    // Transform to homogeneous clip space.
//	matrix wvp = GetWVP();
    
//	Out.Position = mul(float4(In.Position.xyz, 1.0), wvp);
    
//    // Transform to world space space.
//	Out.PositionWS = mul(float4(In.Position.xyz, 1.0f), World);
//	Out.NormalWS = mul(In.Normal.xyz, (float3x3) World);
//	Out.TangentWS = mul(In.Tangent.xyz, (float3x3) World);
    
//	Out.Diffuse = In.Diffuse;
    
//    // Output vertex attributes for interpolation across triangle.
//	Out.TexCoord = In.TexCoord * Parameter.w;
//}