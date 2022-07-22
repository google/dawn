RWByteAddressBuffer sb_rw : register(u0, space0);

uint tint_atomicSub(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedAdd(offset, -value, original_value);
  return original_value;
}


void atomicSub_15bfc9() {
  uint res = tint_atomicSub(sb_rw, 0u, 1u);
}

void fragment_main() {
  atomicSub_15bfc9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicSub_15bfc9();
  return;
}
