int atomicMin_1(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedMin(offset, value, original_value);
  return original_value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicMin_8e38dc() {
  int res = atomicMin_1(sb_rw, 0u, 1);
}

void fragment_main() {
  atomicMin_8e38dc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMin_8e38dc();
  return;
}
