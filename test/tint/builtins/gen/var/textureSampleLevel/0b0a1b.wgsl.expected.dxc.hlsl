SKIP: FAILED

Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleLevel_0b0a1b() {
  float2 arg_2 = (1.0f).xx;
  float arg_3 = 1.0f;
  float4 res = arg_0.SampleLevel(arg_1, arg_2, arg_3, (1).xx);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureSampleLevel_0b0a1b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureSampleLevel_0b0a1b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_0b0a1b();
  return;
}
DXC validation failure:
warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
error: Module bitcode is invalid.
error: Call parameter type does not match function signature!
i64 1
 i32  %4 = call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62, %dx.types.Handle %2, %dx.types.Handle %3, float 1.000000e+00, float 1.000000e+00, float undef, float undef, i64 1, i64 1, i32 undef, float 1.000000e+00)

Validation failed.



warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
error: Module bitcode is invalid.
error: Call parameter type does not match function signature!
i64 1
 i32  %4 = call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62, %dx.types.Handle %2, %dx.types.Handle %3, float 1.000000e+00, float 1.000000e+00, float undef, float undef, i64 1, i64 1, i32 undef, float 1.000000e+00)

Validation failed.



warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
error: Module bitcode is invalid.
error: Call parameter type does not match function signature!
i64 1
 i32  %4 = call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62, %dx.types.Handle %2, %dx.types.Handle %3, float 1.000000e+00, float 1.000000e+00, float undef, float undef, i64 1, i64 1, i32 undef, float 1.000000e+00)

Validation failed.



