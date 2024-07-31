
RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_cdc29e() {
  uint v = 0u;
  sb_rw.InterlockedExchange(uint(0u), 1u, v);
}

void fragment_main() {
  atomicStore_cdc29e();
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_cdc29e();
}

