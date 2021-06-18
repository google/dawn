RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicLoad_fe6cc3() {
  uint atomic_result = 0u;
  sb_rw.InterlockedOr(0u, 0, atomic_result);
  uint res = atomic_result;
}

void fragment_main() {
  atomicLoad_fe6cc3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicLoad_fe6cc3();
  return;
}
