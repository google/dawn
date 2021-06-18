RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicCompareExchangeWeak_6673da() {
  uint2 atomic_result = uint2(0u, 0u);
  uint atomic_compare_value = 1u;
  sb_rw.InterlockedCompareExchange(0u, atomic_compare_value, 1u, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  uint2 res = atomic_result;
}

void fragment_main() {
  atomicCompareExchangeWeak_6673da();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_6673da();
  return;
}
