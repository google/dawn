uint atomicMin_1(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedMin(offset, value, original_value);
  return original_value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicMin_c67a74() {
  uint res = atomicMin_1(sb_rw, 0u, 1u);
}

void fragment_main() {
  atomicMin_c67a74();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMin_c67a74();
  return;
}
