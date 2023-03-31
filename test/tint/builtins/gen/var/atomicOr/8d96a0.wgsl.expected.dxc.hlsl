RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicOr(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedOr(offset, value, original_value);
  return original_value;
}


RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicOr_8d96a0() {
  int arg_1 = 1;
  int res = sb_rwatomicOr(0u, arg_1);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  atomicOr_8d96a0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicOr_8d96a0();
  return;
}
