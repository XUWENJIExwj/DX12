//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer cbPerObject
// {
//
//   float4x4 gWorld;                   // Offset:    0 Size:    64 [unused]
//   float4x4 gTexTransform;            // Offset:   64 Size:    64 [unused]
//   uint gMaterialIndex;               // Offset:  128 Size:     4
//   uint gObjPad0;                     // Offset:  132 Size:     4 [unused]
//   uint gObjPad1;                     // Offset:  136 Size:     4 [unused]
//   uint gObjPad2;                     // Offset:  140 Size:     4 [unused]
//
// }
//
// Resource bind info for gMaterialData
// {
//
//   struct MaterialData
//   {
//       
//       float4 DiffuseAlbedo;          // Offset:    0
//       float4 CubeMapDiffuseAlbedo;   // Offset:   16
//       float3 FresnelR0;              // Offset:   32
//       float Roughness;               // Offset:   44
//       float4x4 MatTransform;         // Offset:   48
//       uint DiffuseMapIndex;          // Offset:  112
//       uint NormalMapIndex;           // Offset:  116
//       uint HeightMapIndex;           // Offset:  120
//       int BitangentSign;             // Offset:  124
//       uint UseACForPOM;              // Offset:  128
//       uint MaxSampleCount;           // Offset:  132
//       uint MinSampleCount;           // Offset:  136
//       uint CascadeDebugIndex;        // Offset:  140
//       float HeightScale;             // Offset:  144
//       float ShadowSoftening;         // Offset:  148
//       float floatPad0;               // Offset:  152
//       float floatPad1;               // Offset:  156
//       bool ShowSelfShadow;           // Offset:  160
//       bool boolPad0;                 // Offset:  164
//       bool boolPad1;                 // Offset:  168
//       bool boolPad2;                 // Offset:  172
//
//   } $Element;                        // Offset:    0 Size:   176
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      ID      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- ------- -------------- ------
// gsamLinearWrap                    sampler      NA          NA      S0             s2      1 
// gCubeMap                          texture  float4        cube      T0             t0      1 
// gMaterialData                     texture  struct         r/o      T1      t0,space2      1 
// cbPerObject                       cbuffer      NA          NA     CB0            cb0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float       
// POSITION                 0   xyz         1     NONE   float   xyz 
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Target                0   xyzw        0   TARGET   float   xyzw
//
ps_5_1
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[0:0][9], immediateIndexed, space=0
dcl_sampler S0[2:2], mode_default, space=0
dcl_resource_texturecube (float,float,float,float) T0[0:0], space=0
dcl_resource_structured T1[0:0], 176, space=2
dcl_input_ps linear v1.xyz
dcl_output o0.xyzw
dcl_temps 2
ld_structured r0.xyz, CB0[0][8].x, l(0), T1[0].xyzx
sample r1.xyzw, v1.xyzx, T0[0].xyzw, S0[2]
mul o0.xyz, r0.xyzx, r1.xyzx
mov o0.w, r1.w
ret 
// Approximately 5 instruction slots used
