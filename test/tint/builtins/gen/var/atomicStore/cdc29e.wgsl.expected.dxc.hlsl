RWByteAddressBuffer sb_rw : register(u0);

void sb_rwatomicStore(uint offset, uint value) {
  uint ignored;
  sb_rw.InterlockedExchange(offset, value, ignored);
}


void atomicStore_cdc29e() {
  uint arg_1 = 1u;
  sb_rwatomicStore(0u, arg_1);
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
