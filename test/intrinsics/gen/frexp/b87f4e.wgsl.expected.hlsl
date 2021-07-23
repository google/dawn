intrinsics/gen/frexp/b87f4e.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<f32> = frexp(vec4<f32>(), &arg_1);
                       ^^^^^

float4 tint_frexp(float4 param_0, inout int4 param_1) {
  float4 float_exp;
  float4 significand = frexp(param_0, float_exp);
  param_1 = int4(float_exp);
  return significand;
}

groupshared int4 arg_1;

void frexp_b87f4e() {
  float4 res = tint_frexp(float4(0.0f, 0.0f, 0.0f, 0.0f), arg_1);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  const uint local_invocation_index = tint_symbol.local_invocation_index;
  if ((local_invocation_index == 0u)) {
    arg_1 = int4(0, 0, 0, 0);
  }
  GroupMemoryBarrierWithGroupSync();
  frexp_b87f4e();
  return;
}
