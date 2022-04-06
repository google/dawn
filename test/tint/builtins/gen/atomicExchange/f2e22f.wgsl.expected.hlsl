int atomicExchange_1(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedExchange(offset, value, original_value);
  return original_value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicExchange_f2e22f() {
  int res = atomicExchange_1(sb_rw, 0u, 1);
}

void fragment_main() {
  atomicExchange_f2e22f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicExchange_f2e22f();
  return;
}
