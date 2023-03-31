RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicMin(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedMin(offset, value, original_value);
  return original_value;
}


RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicMin_8e38dc() {
  int res = sb_rwatomicMin(0u, 1);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  atomicMin_8e38dc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMin_8e38dc();
  return;
}
