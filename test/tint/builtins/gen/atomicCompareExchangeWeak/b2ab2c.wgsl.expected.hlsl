groupshared uint arg_0;

void atomicCompareExchangeWeak_b2ab2c() {
  uint2 atomic_result = uint2(0u, 0u);
  uint atomic_compare_value = 1u;
  InterlockedCompareExchange(arg_0, atomic_compare_value, 1u, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  uint2 res = atomic_result;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    uint atomic_result_1 = 0u;
    InterlockedExchange(arg_0, 0u, atomic_result_1);
  }
  GroupMemoryBarrierWithGroupSync();
  atomicCompareExchangeWeak_b2ab2c();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
