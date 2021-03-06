//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer cbPerObject
// {
//
//   float4x4 gWorld;                   // Offset:    0 Size:    64
//   float4x4 gTexTransform;            // Offset:   64 Size:    64
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
// gMaterialData                     texture  struct         r/o      T0      t0,space2      1 
// cbPerObject                       cbuffer      NA          NA     CB0            cb0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyz         0     NONE   float   xyz 
// TEXCOORD                 0   xy          1     NONE   float   xy  
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float   xyzw
// TEXCOORD                 0   xy          1     NONE   float   xy  
//
vs_5_1
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[0:0][9], immediateIndexed, space=0
dcl_resource_structured T0[0:0], 176, space=2
dcl_input v0.xyz
dcl_input v1.xy
dcl_output_siv o0.xyzw, position
dcl_output o1.xy
dcl_temps 4
ld_structured r0.xyzw, CB0[0][8].x, l(48), T0[0].xyzw
ld_structured r1.xyzw, CB0[0][8].x, l(64), T0[0].xyzw
mov r2.xyz, v0.xyzx
mov r2.w, l(1.000000)
dp4 o0.x, r2.xyzw, CB0[0][0].xyzw
dp4 o0.y, r2.xyzw, CB0[0][1].xyzw
dp4 o0.z, r2.xyzw, CB0[0][2].xyzw
dp4 o0.w, r2.xyzw, CB0[0][3].xyzw
mov r2.xy, v1.xyxx
mov r2.z, l(1.000000)
dp3 r3.x, r2.xyzx, CB0[0][4].xywx
dp3 r3.y, r2.xyzx, CB0[0][5].xywx
dp3 r3.z, r2.xyzx, CB0[0][6].xywx
dp3 r3.w, r2.xyzx, CB0[0][7].xywx
dp4 o1.x, r3.xyzw, r0.xyzw
dp4 o1.y, r3.xyzw, r1.xyzw
ret 
// Approximately 17 instruction slots used
