RWByteAddressBuffer sb_rw : register(u0, space0);

uint sb_rwatomicMin(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedMin(offset, value, original_value);
  return original_value;
}


void atomicMin_c67a74() {
  uint res = sb_rwatomicMin(0u, 1u);
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
