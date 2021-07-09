RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicAdd_8a199a() {
  uint atomic_result = 0u;
  sb_rw.InterlockedAdd(0u, 1u, atomic_result);
  uint res = atomic_result;
}

void fragment_main() {
  atomicAdd_8a199a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAdd_8a199a();
  return;
}
