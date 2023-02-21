static uint local_invocation_index_1 = 0u;
groupshared uint arg_0;

void atomicStore_726882() {
  uint atomic_result = 0u;
  InterlockedExchange(arg_0, 1u, atomic_result);
  return;
}

void compute_main_inner(uint local_invocation_index_2) {
  uint atomic_result_1 = 0u;
  InterlockedExchange(arg_0, 0u, atomic_result_1);
  GroupMemoryBarrierWithGroupSync();
  atomicStore_726882();
  return;
}

void compute_main_1() {
  const uint x_28 = local_invocation_index_1;
  compute_main_inner(x_28);
  return;
}

struct tint_symbol_1 {
  uint local_invocation_index_1_param : SV_GroupIndex;
};

void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    uint atomic_result_2 = 0u;
    InterlockedExchange(arg_0, 0u, atomic_result_2);
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner_1(tint_symbol.local_invocation_index_1_param);
  return;
}
