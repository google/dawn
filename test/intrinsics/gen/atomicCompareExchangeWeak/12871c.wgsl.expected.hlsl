RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicCompareExchangeWeak_12871c() {
  int2 atomic_result = int2(0, 0);
  int atomic_compare_value = 1;
  sb_rw.InterlockedCompareExchange(0u, atomic_compare_value, 1, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  int2 res = atomic_result;
}

void fragment_main() {
  atomicCompareExchangeWeak_12871c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_12871c();
  return;
}
