intrinsics/gen/modf/e38ae6.wgsl:29:18 warning: use of deprecated intrinsic
  var res: f32 = modf(1.0, &arg_1);
                 ^^^^

groupshared float arg_1;

void modf_e38ae6() {
  float res = modf(1.0f, arg_1);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    arg_1 = 0.0f;
  }
  GroupMemoryBarrierWithGroupSync();
  modf_e38ae6();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
