groupshared int arg_0;

void atomicCompareExchangeWeak_89ea3b() {
  int2 atomic_result = int2(0, 0);
  int atomic_compare_value = 1;
  InterlockedCompareExchange(arg_0, atomic_compare_value, 1, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  int2 res = atomic_result;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  const uint local_invocation_index = tint_symbol.local_invocation_index;
  if ((local_invocation_index == 0u)) {
    int atomic_result_1 = 0;
    InterlockedExchange(arg_0, 0, atomic_result_1);
  }
  GroupMemoryBarrierWithGroupSync();
  atomicCompareExchangeWeak_89ea3b();
  return;
}
