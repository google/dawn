RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicExchange(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedExchange(offset, value, original_value);
  return original_value;
}


RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicExchange_d59712() {
  uint arg_1 = 1u;
  uint res = sb_rwatomicExchange(0u, arg_1);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  atomicExchange_d59712();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicExchange_d59712();
  return;
}
