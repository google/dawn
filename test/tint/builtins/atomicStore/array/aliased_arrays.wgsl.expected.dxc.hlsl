groupshared uint wg[3][2][1];

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 6u); idx = (idx + 1u)) {
      const uint i = (idx / 2u);
      const uint i_1 = (idx % 2u);
      const uint i_2 = (idx % 1u);
      uint atomic_result = 0u;
      InterlockedExchange(wg[i][i_1][i_2], 0u, atomic_result);
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_1 = 0u;
  InterlockedExchange(wg[2][1][0], 1u, atomic_result_1);
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
