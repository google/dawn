RWByteAddressBuffer sb_rw : register(u0, space0);

uint sb_rwatomicAdd(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAdd(offset, value, original_value);
  return original_value;
}


void atomicAdd_8a199a() {
  uint res = sb_rwatomicAdd(0u, 1u);
}

void fragment_main() {
  atomicAdd_8a199a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAdd_8a199a();
  return;
}
