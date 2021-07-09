RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicMax_92aa72() {
  int atomic_result = 0;
  sb_rw.InterlockedMax(0u, 1, atomic_result);
  int res = atomic_result;
}

void fragment_main() {
  atomicMax_92aa72();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMax_92aa72();
  return;
}
