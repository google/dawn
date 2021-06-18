groupshared uint arg_0;

void atomicCompareExchangeWeak_b2ab2c() {
  uint2 atomic_result = uint2(0u, 0u);
  uint atomic_compare_value = 1u;
  InterlockedCompareExchange(arg_0, atomic_compare_value, 1u, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  uint2 res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_b2ab2c();
  return;
}
