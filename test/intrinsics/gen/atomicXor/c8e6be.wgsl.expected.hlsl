groupshared uint arg_0;

void atomicXor_c8e6be() {
  uint atomic_result = 0u;
  InterlockedXor(arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  const uint local_invocation_index = tint_symbol.local_invocation_index;
  if ((local_invocation_index == 0u)) {
    uint atomic_result_1 = 0u;
    InterlockedExchange(arg_0, 0u, atomic_result_1);
  }
  GroupMemoryBarrierWithGroupSync();
  atomicXor_c8e6be();
  return;
}
