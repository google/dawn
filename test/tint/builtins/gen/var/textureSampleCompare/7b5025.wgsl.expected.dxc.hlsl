SKIP: FAILED

Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleCompare_7b5025() {
  float2 arg_2 = (1.0f).xx;
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  float res = arg_0.SampleCmp(arg_1, float3(arg_2, float(arg_3)), arg_4, (1).xx);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  textureSampleCompare_7b5025();
  return;
}
DXC validation failure:
warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
error: Module bitcode is invalid.
error: Call parameter type does not match function signature!
i64 1
 i32  %4 = call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64, %dx.types.Handle %2, %dx.types.Handle %3, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float undef, i64 1, i64 1, i32 undef, float 1.000000e+00, float undef)

Validation failed.



