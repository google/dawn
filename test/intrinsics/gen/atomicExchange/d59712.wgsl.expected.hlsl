RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicExchange_d59712() {
  uint atomic_result = 0u;
  sb_rw.InterlockedExchange(0u, 1u, atomic_result);
  uint res = atomic_result;
}

void fragment_main() {
  atomicExchange_d59712();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicExchange_d59712();
  return;
}
