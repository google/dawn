int atomicAdd_1(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicAdd_d32fe4() {
  int res = atomicAdd_1(sb_rw, 0u, 1);
}

void fragment_main() {
  atomicAdd_d32fe4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAdd_d32fe4();
  return;
}
