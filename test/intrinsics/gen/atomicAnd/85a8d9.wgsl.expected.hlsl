RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicAnd_85a8d9() {
  uint atomic_result = 0u;
  sb_rw.InterlockedAnd(0u, 0u, atomic_result);
  uint res = atomic_result;
}

void fragment_main() {
  atomicAnd_85a8d9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAnd_85a8d9();
  return;
}
