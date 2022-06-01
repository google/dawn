RWByteAddressBuffer sb_rw : register(u0, space0);

void tint_atomicStore(RWByteAddressBuffer buffer, uint offset, int value) {
  int ignored;
  buffer.InterlockedExchange(offset, value, ignored);
}


void atomicStore_d1e9a6() {
  tint_atomicStore(sb_rw, 0u, 1);
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
