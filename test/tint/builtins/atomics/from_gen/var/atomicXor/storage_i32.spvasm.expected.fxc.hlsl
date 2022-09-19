RWByteAddressBuffer sb_rw : register(u0, space0);

int tint_atomicXor(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedXor(offset, value, original_value);
  return original_value;
}


void atomicXor_c1b78c() {
  int arg_1 = 0;
  int res = 0;
  arg_1 = 1;
  const int x_20 = arg_1;
  const int x_13 = tint_atomicXor(sb_rw, 0u, x_20);
  res = x_13;
  return;
}

void fragment_main_1() {
  atomicXor_c1b78c();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicXor_c1b78c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
