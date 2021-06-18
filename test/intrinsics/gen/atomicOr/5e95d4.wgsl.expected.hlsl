RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicOr_5e95d4() {
  uint atomic_result = 0u;
  sb_rw.InterlockedOr(0u, 0u, atomic_result);
  uint res = atomic_result;
}

void fragment_main() {
  atomicOr_5e95d4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicOr_5e95d4();
  return;
}
