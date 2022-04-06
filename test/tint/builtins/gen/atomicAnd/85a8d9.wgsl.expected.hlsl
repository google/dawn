uint atomicAnd_1(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedAnd(offset, value, original_value);
  return original_value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicAnd_85a8d9() {
  uint res = atomicAnd_1(sb_rw, 0u, 1u);
}

void fragment_main() {
  atomicAnd_85a8d9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAnd_85a8d9();
  return;
}
