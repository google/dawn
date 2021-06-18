RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicStore_cdc29e() {
  uint atomic_result = 0u;
  sb_rw.InterlockedExchange(0u, 1u, atomic_result);
}

void fragment_main() {
  atomicStore_cdc29e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_cdc29e();
  return;
}
