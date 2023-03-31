RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicAnd(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedAnd(offset, value, original_value);
  return original_value;
}


RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicAnd_152966() {
  int arg_1 = 1;
  int res = sb_rwatomicAnd(0u, arg_1);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  atomicAnd_152966();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAnd_152966();
  return;
}
