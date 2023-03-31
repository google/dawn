RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicMin(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedMin(offset, value, original_value);
  return original_value;
}


RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicMin_c67a74() {
  uint arg_1 = 1u;
  uint res = sb_rwatomicMin(0u, arg_1);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  atomicMin_c67a74();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMin_c67a74();
  return;
}
