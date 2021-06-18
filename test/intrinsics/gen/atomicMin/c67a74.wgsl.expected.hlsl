RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicMin_c67a74() {
  uint atomic_result = 0u;
  sb_rw.InterlockedMin(0u, 0u, atomic_result);
  uint res = atomic_result;
}

void fragment_main() {
  atomicMin_c67a74();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMin_c67a74();
  return;
}
