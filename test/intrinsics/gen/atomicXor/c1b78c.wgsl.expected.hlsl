RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicXor_c1b78c() {
  int atomic_result = 0;
  sb_rw.InterlockedXor(0u, 0u, atomic_result);
  int res = atomic_result;
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
