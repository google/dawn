
RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_cdc29e() {
  uint arg_1 = 0u;
  arg_1 = 1u;
  uint x_18 = arg_1;
  uint v = 0u;
  sb_rw.InterlockedExchange(uint(0u), x_18, v);
}

void fragment_main_1() {
  atomicStore_cdc29e();
}

void fragment_main() {
  fragment_main_1();
}

void compute_main_1() {
  atomicStore_cdc29e();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

