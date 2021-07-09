RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicMin_8e38dc() {
  int atomic_result = 0;
  sb_rw.InterlockedMin(0u, 1, atomic_result);
  int res = atomic_result;
}

void fragment_main() {
  atomicMin_8e38dc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMin_8e38dc();
  return;
}
