intrinsics/gen/frexp/40fc9b.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec3<f32> = frexp(vec3<f32>(), &arg_1);
                       ^^^^^

float3 tint_frexp(float3 param_0, inout int3 param_1) {
  float3 float_exp;
  float3 significand = frexp(param_0, float_exp);
  param_1 = int3(float_exp);
  return significand;
}

groupshared int3 arg_1;

void frexp_40fc9b() {
  float3 res = tint_frexp(float3(0.0f, 0.0f, 0.0f), arg_1);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  const uint local_invocation_index = tint_symbol.local_invocation_index;
  {
    arg_1 = int3(0, 0, 0);
  }
  GroupMemoryBarrierWithGroupSync();
  frexp_40fc9b();
  return;
}
