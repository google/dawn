RWByteAddressBuffer sb_rw : register(u0, space0);

void tint_atomicStore(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint ignored;
  buffer.InterlockedExchange(offset, value, ignored);
}


void atomicStore_cdc29e() {
  tint_atomicStore(sb_rw, 0u, 1u);
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
