RWByteAddressBuffer sb_rw : register(u0, space0);

int sb_rwatomicXor(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedXor(offset, value, original_value);
  return original_value;
}


void atomicXor_c1b78c() {
  int arg_1 = 1;
  int res = sb_rwatomicXor(0u, arg_1);
}

void fragment_main() {
  atomicXor_c1b78c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicXor_c1b78c();
  return;
}
