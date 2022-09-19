RWByteAddressBuffer sb_rw : register(u0, space0);

void tint_atomicStore(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint ignored;
  buffer.InterlockedExchange(offset, value, ignored);
}


void atomicStore_cdc29e() {
  uint arg_1 = 0u;
  arg_1 = 1u;
  const uint x_18 = arg_1;
  tint_atomicStore(sb_rw, 0u, x_18);
  return;
}

void fragment_main_1() {
  atomicStore_cdc29e();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicStore_cdc29e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
