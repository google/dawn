RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicXor_54510e() {
  uint atomic_result = 0u;
  sb_rw.InterlockedXor(0u, 1u, atomic_result);
  uint res = atomic_result;
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
