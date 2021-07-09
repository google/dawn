uint atomicXor_1(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedXor(offset, value, original_value);
  return original_value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicXor_54510e() {
  uint res = atomicXor_1(sb_rw, 0u, 1u);
}

void fragment_main() {
  atomicXor_54510e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicXor_54510e();
  return;
}
