RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicAnd(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAnd(offset, value, original_value);
  return original_value;
}


RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicAnd_85a8d9() {
  uint arg_1 = 1u;
  uint res = sb_rwatomicAnd(0u, arg_1);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  atomicAnd_85a8d9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAnd_85a8d9();
  return;
}
