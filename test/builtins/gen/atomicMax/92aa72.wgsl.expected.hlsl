int atomicMax_1(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedMax(offset, value, original_value);
  return original_value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicMax_92aa72() {
  int res = atomicMax_1(sb_rw, 0u, 1);
}

void fragment_main() {
  atomicMax_92aa72();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMax_92aa72();
  return;
}
