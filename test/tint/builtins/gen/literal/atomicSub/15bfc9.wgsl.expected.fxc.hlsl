RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicSub(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAdd(offset, -value, original_value);
  return original_value;
}


RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicSub_15bfc9() {
  uint res = sb_rwatomicSub(0u, 1u);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  atomicSub_15bfc9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicSub_15bfc9();
  return;
}
