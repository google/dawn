int atomicXor_1(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedXor(offset, value, original_value);
  return original_value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicXor_c1b78c() {
  int res = atomicXor_1(sb_rw, 0u, 1);
}

void fragment_main() {
  atomicXor_c1b78c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicXor_c1b78c();
  return;
}
