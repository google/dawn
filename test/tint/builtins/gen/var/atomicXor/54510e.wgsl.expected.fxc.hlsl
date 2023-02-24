RWByteAddressBuffer sb_rw : register(u0, space0);

uint sb_rwatomicXor(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedXor(offset, value, original_value);
  return original_value;
}


void atomicXor_54510e() {
  uint arg_1 = 1u;
  uint res = sb_rwatomicXor(0u, arg_1);
}

void fragment_main() {
  atomicXor_54510e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicXor_54510e();
  return;
}
