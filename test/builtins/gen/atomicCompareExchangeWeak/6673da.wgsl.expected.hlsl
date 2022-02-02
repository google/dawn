uint2 atomicCompareExchangeWeak_1(RWByteAddressBuffer buffer, uint offset, uint compare, uint value) {
  uint2 result = {0, 0};
  buffer.InterlockedCompareExchange(offset, compare, value, result.x);
  result.y = result.x == compare;
  return result;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicCompareExchangeWeak_6673da() {
  uint2 res = atomicCompareExchangeWeak_1(sb_rw, 0u, 1u, 1u);
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
