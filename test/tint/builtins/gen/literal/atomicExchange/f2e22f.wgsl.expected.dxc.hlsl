RWByteAddressBuffer sb_rw : register(u0, space0);

int sb_rwatomicExchange(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedExchange(offset, value, original_value);
  return original_value;
}


void atomicExchange_f2e22f() {
  int res = sb_rwatomicExchange(0u, 1);
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
