RWByteAddressBuffer sb_rw : register(u0, space0);

uint sb_rwatomicSub(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAdd(offset, -value, original_value);
  return original_value;
}


void atomicSub_15bfc9() {
  uint arg_1 = 1u;
  uint res = sb_rwatomicSub(0u, arg_1);
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
