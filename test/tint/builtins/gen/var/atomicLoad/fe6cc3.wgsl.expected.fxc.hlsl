RWByteAddressBuffer sb_rw : register(u0, space0);

uint sb_rwatomicLoad(uint offset) {
  uint value = 0;
  sb_rw.InterlockedOr(offset, 0, value);
  return value;
}


void atomicLoad_fe6cc3() {
  uint res = sb_rwatomicLoad(0u);
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
