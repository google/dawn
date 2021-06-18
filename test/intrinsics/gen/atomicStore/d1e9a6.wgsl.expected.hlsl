RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicStore_d1e9a6() {
  int atomic_result = 0;
  sb_rw.InterlockedExchange(0u, 1, atomic_result);
}

void fragment_main() {
  atomicStore_d1e9a6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_d1e9a6();
  return;
}
