uint atomicLoad_1(RWByteAddressBuffer buffer, uint offset) {
  uint value = 0;
  buffer.InterlockedOr(offset, 0, value);
  return value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicLoad_fe6cc3() {
  uint res = atomicLoad_1(sb_rw, 0u);
}

void fragment_main() {
  atomicLoad_fe6cc3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicLoad_fe6cc3();
  return;
}
