groupshared float16_t arg_0;

float16_t tint_workgroupUniformLoad_arg_0() {
  GroupMemoryBarrierWithGroupSync();
  const float16_t result = arg_0;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

void workgroupUniformLoad_e07d08() {
  float16_t res = tint_workgroupUniformLoad_arg_0();
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    arg_0 = float16_t(0.0h);
  }
  GroupMemoryBarrierWithGroupSync();
  workgroupUniformLoad_e07d08();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
