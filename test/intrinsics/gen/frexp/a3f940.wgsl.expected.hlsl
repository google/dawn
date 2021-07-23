intrinsics/gen/frexp/a3f940.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec2<f32> = frexp(vec2<f32>(), &arg_1);
                       ^^^^^

float2 tint_frexp(float2 param_0, inout int2 param_1) {
  float2 float_exp;
  float2 significand = frexp(param_0, float_exp);
  param_1 = int2(float_exp);
  return significand;
}

groupshared int2 arg_1;

void frexp_a3f940() {
  float2 res = tint_frexp(float2(0.0f, 0.0f), arg_1);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  const uint local_invocation_index = tint_symbol.local_invocation_index;
  if ((local_invocation_index == 0u)) {
    arg_1 = int2(0, 0);
  }
  GroupMemoryBarrierWithGroupSync();
  frexp_a3f940();
  return;
}
