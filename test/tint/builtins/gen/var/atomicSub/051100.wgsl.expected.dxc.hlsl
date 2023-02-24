RWByteAddressBuffer sb_rw : register(u0, space0);

int sb_rwatomicSub(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedAdd(offset, -value, original_value);
  return original_value;
}


void atomicSub_051100() {
  int arg_1 = 1;
  int res = sb_rwatomicSub(0u, arg_1);
}

void fragment_main() {
  atomicSub_051100();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicSub_051100();
  return;
}
