RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicLoad_0806ad() {
  int atomic_result = 0;
  sb_rw.InterlockedOr(0u, 0, atomic_result);
  int res = atomic_result;
}

void fragment_main() {
  atomicLoad_0806ad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicLoad_0806ad();
  return;
}
