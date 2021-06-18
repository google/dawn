RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicOr_8d96a0() {
  int atomic_result = 0;
  sb_rw.InterlockedOr(0u, 0u, atomic_result);
  int res = atomic_result;
}

void fragment_main() {
  atomicOr_8d96a0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicOr_8d96a0();
  return;
}
