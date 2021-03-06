//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer cbSettings
// {
//
//   int gBlurRadius;                   // Offset:    0 Size:     4
//   float w00;                         // Offset:    4 Size:     4
//   float w01;                         // Offset:    8 Size:     4
//   float w02;                         // Offset:   12 Size:     4
//   float w03;                         // Offset:   16 Size:     4
//   float w04;                         // Offset:   20 Size:     4
//   float w05;                         // Offset:   24 Size:     4
//   float w06;                         // Offset:   28 Size:     4
//   float w07;                         // Offset:   32 Size:     4
//   float w08;                         // Offset:   36 Size:     4
//   float w09;                         // Offset:   40 Size:     4
//   float w10;                         // Offset:   44 Size:     4
//   float w11;                         // Offset:   48 Size:     4
//   float w12;                         // Offset:   52 Size:     4
//   float w13;                         // Offset:   56 Size:     4
//   float w14;                         // Offset:   60 Size:     4
//   float w15;                         // Offset:   64 Size:     4
//   float w16;                         // Offset:   68 Size:     4
//   float w17;                         // Offset:   72 Size:     4
//   float w18;                         // Offset:   76 Size:     4
//   float w19;                         // Offset:   80 Size:     4
//   float w20;                         // Offset:   84 Size:     4
//   float w21;                         // Offset:   88 Size:     4
//   float w22;                         // Offset:   92 Size:     4
//   float w23;                         // Offset:   96 Size:     4
//   float w24;                         // Offset:  100 Size:     4
//   float w25;                         // Offset:  104 Size:     4
//   float w26;                         // Offset:  108 Size:     4
//   float w27;                         // Offset:  112 Size:     4
//   float w28;                         // Offset:  116 Size:     4
//   float w29;                         // Offset:  120 Size:     4
//   float w30;                         // Offset:  124 Size:     4
//   float w31;                         // Offset:  128 Size:     4
//   float w32;                         // Offset:  132 Size:     4
//   float w33;                         // Offset:  136 Size:     4
//   float w34;                         // Offset:  140 Size:     4
//   float w35;                         // Offset:  144 Size:     4
//   float w36;                         // Offset:  148 Size:     4
//   float w37;                         // Offset:  152 Size:     4
//   float w38;                         // Offset:  156 Size:     4
//   float w39;                         // Offset:  160 Size:     4
//   float w40;                         // Offset:  164 Size:     4
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// gInput                            texture  float4          2d             t0      1 
// gOutput                               UAV  float4          2d             u0      1 
// cbSettings                        cbuffer      NA          NA            cb0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Input
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Output
cs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[11], immediateIndexed
dcl_resource_texture2d (float,float,float,float) t0
dcl_uav_typed_texture2d (float,float,float,float) u0
dcl_input vThreadIDInGroup.y
dcl_input vThreadID.xy
dcl_temps 4
dcl_indexableTemp x0[41], 4
dcl_tgsm_structured g0, 16, 296
dcl_thread_group 1, 256, 1
mov x0[0].x, cb0[0].y
mov x0[1].x, cb0[0].z
mov x0[2].x, cb0[0].w
mov x0[3].x, cb0[1].x
mov x0[4].x, cb0[1].y
mov x0[5].x, cb0[1].z
mov x0[6].x, cb0[1].w
mov x0[7].x, cb0[2].x
mov x0[8].x, cb0[2].y
mov x0[9].x, cb0[2].z
mov x0[10].x, cb0[2].w
mov x0[11].x, cb0[3].x
mov x0[12].x, cb0[3].y
mov x0[13].x, cb0[3].z
mov x0[14].x, cb0[3].w
mov x0[15].x, cb0[4].x
mov x0[16].x, cb0[4].y
mov x0[17].x, cb0[4].z
mov x0[18].x, cb0[4].w
mov x0[19].x, cb0[5].x
mov x0[20].x, cb0[5].y
mov x0[21].x, cb0[5].z
mov x0[22].x, cb0[5].w
mov x0[23].x, cb0[6].x
mov x0[24].x, cb0[6].y
mov x0[25].x, cb0[6].z
mov x0[26].x, cb0[6].w
mov x0[27].x, cb0[7].x
mov x0[28].x, cb0[7].y
mov x0[29].x, cb0[7].z
mov x0[30].x, cb0[7].w
mov x0[31].x, cb0[8].x
mov x0[32].x, cb0[8].y
mov x0[33].x, cb0[8].z
mov x0[34].x, cb0[8].w
mov x0[35].x, cb0[9].x
mov x0[36].x, cb0[9].y
mov x0[37].x, cb0[9].z
mov x0[38].x, cb0[9].w
mov x0[39].x, cb0[10].x
mov x0[40].x, cb0[10].y
ilt r0.x, vThreadIDInGroup.y, cb0[0].x
if_nz r0.x
  iadd r0.x, vThreadID.y, -cb0[0].x
  imax r0.y, r0.x, l(0)
  mov r0.x, vThreadID.x
  mov r0.zw, l(0,0,0,0)
  ld_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyzw, t0.xyzw
  store_structured g0.xyzw, vThreadIDInGroup.y, l(0), r0.xyzw
endif 
ineg r0.x, cb0[0].x
iadd r0.y, r0.x, l(256)
ige r0.y, vThreadIDInGroup.y, r0.y
if_nz r0.y
  iadd r0.y, vThreadID.y, cb0[0].x
  resinfo_indexable(texture2d)(float,float,float,float)_uint r0.z, l(0), t0.xzyw
  iadd r0.z, r0.z, l(-1)
  umin r1.y, r0.z, r0.y
  ishl r0.y, cb0[0].x, l(1)
  iadd r0.y, r0.y, vThreadIDInGroup.y
  mov r1.x, vThreadID.x
  mov r1.zw, l(0,0,0,0)
  ld_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyzw, t0.xyzw
  store_structured g0.xyzw, r0.y, l(0), r1.xyzw
endif 
iadd r0.y, vThreadIDInGroup.y, cb0[0].x
resinfo_indexable(texture2d)(float,float,float,float)_uint r0.zw, l(0), t0.zwxy
iadd r0.zw, r0.zzzw, l(0, 0, -1, -1)
umin r1.xy, r0.zwzz, vThreadID.xyxx
mov r1.zw, l(0,0,0,0)
ld_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyzw, t0.xyzw
store_structured g0.xyzw, r0.y, l(0), r1.xyzw
sync_g_t
mov r1.xyzw, l(0,0,0,0)
mov r0.z, r0.x
loop 
  ilt r0.w, cb0[0].x, r0.z
  breakc_nz r0.w
  iadd r0.w, r0.z, r0.y
  iadd r2.x, r0.z, cb0[0].x
  mov r2.x, x0[r2.x + 0].x
  ld_structured r3.xyzw, r0.w, l(0), g0.xyzw
  mad r1.xyzw, r2.xxxx, r3.xyzw, r1.xyzw
  iadd r0.z, r0.z, l(1)
endloop 
store_uav_typed u0.xyzw, vThreadID.xyyy, r1.xyzw
ret 
// Approximately 87 instruction slots used
