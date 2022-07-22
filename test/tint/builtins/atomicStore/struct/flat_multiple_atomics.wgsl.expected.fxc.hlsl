struct S {
  int x;
  uint a;
  uint b;
};

groupshared S wg;

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    wg.x = 0;
    uint atomic_result = 0u;
    InterlockedExchange(wg.a, 0u, atomic_result);
    uint atomic_result_1 = 0u;
    InterlockedExchange(wg.b, 0u, atomic_result_1);
  }
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_2 = 0u;
  InterlockedExchange(wg.a, 1u, atomic_result_2);
  uint atomic_result_3 = 0u;
  InterlockedExchange(wg.b, 2u, atomic_result_3);
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
