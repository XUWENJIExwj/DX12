//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// gInputA                           texture  float4          2d             t0      1 
// gInputB                           texture  float4          2d             t1      1 
// gOutput                               UAV  float4          2d             u0      1 
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
dcl_resource_texture2d (float,float,float,float) t0
dcl_resource_texture2d (float,float,float,float) t1
dcl_uav_typed_texture2d (float,float,float,float) u0
dcl_input vThreadID.xy
dcl_temps 2
dcl_thread_group 16, 16, 1
mov r0.xy, vThreadID.xyxx
mov r0.zw, l(0,0,0,0)
ld_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyww, t0.xyzw
ld_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyzw, t1.xyzw
add r0.xyzw, r0.xyzw, r1.xyzw
store_uav_typed u0.xyzw, vThreadID.xyyy, r0.xyzw
ret 
// Approximately 7 instruction slots used