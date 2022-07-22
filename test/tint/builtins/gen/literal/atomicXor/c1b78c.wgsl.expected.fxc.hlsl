RWByteAddressBuffer sb_rw : register(u0, space0);

int tint_atomicXor(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedXor(offset, value, original_value);
  return original_value;
}


void atomicXor_c1b78c() {
  int res = tint_atomicXor(sb_rw, 0u, 1);
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
