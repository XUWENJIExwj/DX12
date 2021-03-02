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
// cbuffer cbPass
// {
//
//   float4x4 gView;                    // Offset:    0 Size:    64
//   float4x4 gInvView;                 // Offset:   64 Size:    64 [unused]
//   float4x4 gProj;                    // Offset:  128 Size:    64 [unused]
//   float4x4 gInvProj;                 // Offset:  192 Size:    64 [unused]
//   float4x4 gViewProj;                // Offset:  256 Size:    64
//   float4x4 gInvViewProj;             // Offset:  320 Size:    64 [unused]
//   float3 gEyePosWS;                  // Offset:  384 Size:    12 [unused]
//   float cbPerObjectPad1;             // Offset:  396 Size:     4 [unused]
//   float2 gRenderTargetSize;          // Offset:  400 Size:     8 [unused]
//   float2 gInvRenderTargetSize;       // Offset:  408 Size:     8 [unused]
//   float gNearZ;                      // Offset:  416 Size:     4 [unused]
//   float gFarZ;                       // Offset:  420 Size:     4 [unused]
//   float gTotalTime;                  // Offset:  424 Size:     4 [unused]
//   float gDeltaTime;                  // Offset:  428 Size:     4 [unused]
//   float4 gAmbientLight;              // Offset:  432 Size:    16 [unused]
//   
//   struct Light
//   {
//       
//       float3 Strength;               // Offset:  448
//       float FalloffStart;            // Offset:  460
//       float3 Direction;              // Offset:  464
//       float FalloffEnd;              // Offset:  476
//       float3 Position;               // Offset:  480
//       float SpotPower;               // Offset:  492
//
//   } gLights[16];                     // Offset:  448 Size:   768 [unused]
//   float4x4 gShadowView;              // Offset: 1216 Size:    64
//   float gMaxBorderPadding;           // Offset: 1280 Size:     4 [unused]
//   float gMinBorderPadding;           // Offset: 1284 Size:     4 [unused]
//   float gCascadeBlendArea;           // Offset: 1288 Size:     4 [unused]
//   float cbPerObjectPad0;             // Offset: 1292 Size:     4 [unused]
//   float4 gShadowBias;                // Offset: 1296 Size:    16 [unused]
//   float4 gShadowTexScale[3];         // Offset: 1312 Size:    48 [unused]
//   float4 gShadowTexOffset[3];        // Offset: 1360 Size:    48 [unused]
//   int gPCFBlurForLoopStart;          // Offset: 1408 Size:     4 [unused]
//   int gPCFBlurForLoopEnd;            // Offset: 1412 Size:     4 [unused]
//   int cbPerObjectPad2;               // Offset: 1416 Size:     4 [unused]
//   int cbPerObjectPad3;               // Offset: 1420 Size:     4 [unused]
//   bool gVisualCascade;               // Offset: 1424 Size:     4 [unused]
//   bool gBlendCascade;                // Offset: 1428 Size:     4 [unused]
//   bool cbPerObjectPad5;              // Offset: 1432 Size:     4 [unused]
//   bool cbPerObjectPad6;              // Offset: 1436 Size:     4 [unused]
//   float4 gCascadeFrustumsEyeSpaceDepthsFloat;// Offset: 1440 Size:    16 [unused]
//   float4 gCascadeFrustumsEyeSpaceDepthsFloat4[3];// Offset: 1456 Size:    48 [unused]
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
// cbPass                            cbuffer      NA          NA     CB1            cb1      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyz         0     NONE   float   xyz 
// NORMAL                   0   xyz         1     NONE   float   xyz 
// TANGENT                  0   xyz         2     NONE   float   xyz 
// TEXCOORD                 0   xy          3     NONE   float   xy  
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float   xyzw
// POSITION                 0   xyz         1     NONE   float   xyz 
// POSITION                 1   xyzw        2     NONE   float   xyzw
// NORMAL                   0   xyz         3     NONE   float   xyz 
// TANGENT                  0   xyz         4     NONE   float   xyz 
// TEXCOORD                 0   xy          5     NONE   float   xy  
// TEXCOORD                 1     z         5     NONE   float     z 
//
vs_5_1
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[0:0][9], immediateIndexed, space=0
dcl_constantbuffer CB1[1:1][80], immediateIndexed, space=0
dcl_resource_structured T0[0:0], 176, space=2
dcl_input v0.xyz
dcl_input v1.xyz
dcl_input v2.xyz
dcl_input v3.xy
dcl_output_siv o0.xyzw, position
dcl_output o1.xyz
dcl_output o2.xyzw
dcl_output o3.xyz
dcl_output o4.xyz
dcl_output o5.xy
dcl_output o5.z
dcl_temps 5
ld_structured r0.xyzw, CB0[0][8].x, l(48), T0[0].xyzw
ld_structured r1.xyzw, CB0[0][8].x, l(64), T0[0].xyzw
mov r2.xyz, v0.xyzx
mov r2.w, l(1.000000)
dp4 r3.x, r2.xyzw, CB0[0][0].xyzw
dp4 r3.y, r2.xyzw, CB0[0][1].xyzw
dp4 r3.z, r2.xyzw, CB0[0][2].xyzw
dp4 r3.w, r2.xyzw, CB0[0][3].xyzw
dp3 o3.x, v1.xyzx, CB0[0][0].xyzx
dp3 o3.y, v1.xyzx, CB0[0][1].xyzx
dp3 o3.z, v1.xyzx, CB0[0][2].xyzx
dp3 o4.x, v2.xyzx, CB0[0][0].xyzx
dp3 o4.y, v2.xyzx, CB0[0][1].xyzx
dp3 o4.z, v2.xyzx, CB0[0][2].xyzx
dp4 o0.x, r3.xyzw, CB1[1][16].xyzw
dp4 o0.y, r3.xyzw, CB1[1][17].xyzw
dp4 o0.z, r3.xyzw, CB1[1][18].xyzw
dp4 o0.w, r3.xyzw, CB1[1][19].xyzw
mov r2.xy, v3.xyxx
mov r2.z, l(1.000000)
dp3 r4.x, r2.xyzx, CB0[0][4].xywx
dp3 r4.y, r2.xyzx, CB0[0][5].xywx
dp3 r4.z, r2.xyzx, CB0[0][6].xywx
dp3 r4.w, r2.xyzx, CB0[0][7].xywx
dp4 o5.x, r4.xyzw, r0.xyzw
dp4 o5.y, r4.xyzw, r1.xyzw
dp4 o2.x, r3.xyzw, CB1[1][76].xyzw
dp4 o2.y, r3.xyzw, CB1[1][77].xyzw
dp4 o2.z, r3.xyzw, CB1[1][78].xyzw
dp4 o2.w, r3.xyzw, CB1[1][79].xyzw
dp4 o5.z, r3.xyzw, CB1[1][2].xyzw
mov o1.xyz, r3.xyzx
ret 
// Approximately 33 instruction slots used
