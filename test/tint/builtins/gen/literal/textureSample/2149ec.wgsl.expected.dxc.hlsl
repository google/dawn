SKIP: FAILED

Texture3D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSample_2149ec() {
  float4 res = arg_0.Sample(arg_1, (1.0f).xxx, (1).xxx);
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  textureSample_2149ec();
  return;
}
DXC validation failure:
warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
error: Module bitcode is invalid.
error: Call parameter type does not match function signature!
i64 1
 i32  %4 = call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60, %dx.types.Handle %2, %dx.types.Handle %3, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float undef, i64 1, i64 1, i64 1, float undef)

Validation failed.



