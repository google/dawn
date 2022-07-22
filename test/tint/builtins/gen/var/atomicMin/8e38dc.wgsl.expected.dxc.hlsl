RWByteAddressBuffer sb_rw : register(u0, space0);

int tint_atomicMin(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedMin(offset, value, original_value);
  return original_value;
}


void atomicMin_8e38dc() {
  int arg_1 = 1;
  int res = tint_atomicMin(sb_rw, 0u, arg_1);
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
