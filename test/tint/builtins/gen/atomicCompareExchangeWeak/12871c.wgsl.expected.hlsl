int2 atomicCompareExchangeWeak_1(RWByteAddressBuffer buffer, uint offset, int compare, int value) {
  int2 result = {0, 0};
  buffer.InterlockedCompareExchange(offset, compare, value, result.x);
  result.y = result.x == compare;
  return result;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicCompareExchangeWeak_12871c() {
  int2 res = atomicCompareExchangeWeak_1(sb_rw, 0u, 1, 1);
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
