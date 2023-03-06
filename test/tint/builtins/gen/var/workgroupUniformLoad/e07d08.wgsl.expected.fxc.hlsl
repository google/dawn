SKIP: FAILED

float16_t tint_workgroupUniformLoad(inout float16_t p) {
  GroupMemoryBarrierWithGroupSync();
  const float16_t result = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared float16_t arg_0;
RWByteAddressBuffer prevent_dce : register(u0, space2);

void workgroupUniformLoad_e07d08() {
  float16_t res = tint_workgroupUniformLoad(arg_0);
  prevent_dce.Store<float16_t>(0u, res);
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
