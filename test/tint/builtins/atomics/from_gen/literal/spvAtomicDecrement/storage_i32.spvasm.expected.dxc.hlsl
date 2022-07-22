RWByteAddressBuffer sb_rw : register(u0, space0);

int tint_atomicSub(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedAdd(offset, -value, original_value);
  return original_value;
}


void atomicAdd_d32fe4() {
  int res = 0;
  const int x_9 = tint_atomicSub(sb_rw, 0u, 1);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicAdd_d32fe4();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicAdd_d32fe4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
