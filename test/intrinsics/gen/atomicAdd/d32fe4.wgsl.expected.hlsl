RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicAdd_d32fe4() {
  int atomic_result = 0;
  sb_rw.InterlockedAdd(0u, 1, atomic_result);
  int res = atomic_result;
}

void fragment_main() {
  atomicAdd_d32fe4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAdd_d32fe4();
  return;
}
