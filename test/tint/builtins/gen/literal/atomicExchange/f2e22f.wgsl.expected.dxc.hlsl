RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicExchange(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedExchange(offset, value, original_value);
  return original_value;
}


RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicExchange_f2e22f() {
  int res = sb_rwatomicExchange(0u, 1);
  prevent_dce.Store(0u, asuint(res));
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
