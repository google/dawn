intrinsics/gen/modf/a128ab.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec2<f32> = modf(vec2<f32>(), &arg_1);
                       ^^^^

groupshared float2 arg_1;

void modf_a128ab() {
  float2 res = modf(float2(0.0f, 0.0f), arg_1);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  const uint local_invocation_index = tint_symbol.local_invocation_index;
  if ((local_invocation_index == 0u)) {
    arg_1 = float2(0.0f, 0.0f);
  }
  GroupMemoryBarrierWithGroupSync();
  modf_a128ab();
  return;
}
