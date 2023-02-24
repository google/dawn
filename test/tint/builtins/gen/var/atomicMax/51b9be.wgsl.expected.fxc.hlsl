RWByteAddressBuffer sb_rw : register(u0, space0);

uint sb_rwatomicMax(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedMax(offset, value, original_value);
  return original_value;
}


void atomicMax_51b9be() {
  uint arg_1 = 1u;
  uint res = sb_rwatomicMax(0u, arg_1);
}

void fragment_main() {
  atomicMax_51b9be();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMax_51b9be();
  return;
}
