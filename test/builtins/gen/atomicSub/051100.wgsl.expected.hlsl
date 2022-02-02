int atomicSub_1(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedAdd(offset, -value, original_value);
  return original_value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicSub_051100() {
  int res = atomicSub_1(sb_rw, 0u, 1);
}

void fragment_main() {
  atomicSub_051100();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicSub_051100();
  return;
}
