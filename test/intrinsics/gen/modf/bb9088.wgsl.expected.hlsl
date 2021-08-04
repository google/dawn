intrinsics/gen/modf/bb9088.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec3<f32> = modf(vec3<f32>(), &arg_1);
                       ^^^^

groupshared float3 arg_1;

void modf_bb9088() {
  float3 res = modf(float3(0.0f, 0.0f, 0.0f), arg_1);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    arg_1 = float3(0.0f, 0.0f, 0.0f);
  }
  GroupMemoryBarrierWithGroupSync();
  modf_bb9088();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
