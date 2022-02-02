void atomicStore_1(RWByteAddressBuffer buffer, uint offset, int value) {
  int ignored;
  buffer.InterlockedExchange(offset, value, ignored);
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicStore_d1e9a6() {
  atomicStore_1(sb_rw, 0u, 1);
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
