
RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_cdc29e() {
  uint arg_1 = 1u;
  uint v = arg_1;
  uint v_1 = 0u;
  sb_rw.InterlockedExchange(uint(0u), v, v_1);
}

void fragment_main() {
  atomicStore_cdc29e();
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_cdc29e();
}

