RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicMax_51b9be() {
  uint atomic_result = 0u;
  sb_rw.InterlockedMax(0u, 1u, atomic_result);
  uint res = atomic_result;
}

void fragment_main() {
  atomicMax_51b9be();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMax_51b9be();
  return;
}
