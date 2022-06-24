RWByteAddressBuffer sb_rw : register(u0, space0);

int tint_atomicAnd(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedAnd(offset, value, original_value);
  return original_value;
}


void atomicAnd_152966() {
  int res = 0;
  const int x_9 = tint_atomicAnd(sb_rw, 0u, 1);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicAnd_152966();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicAnd_152966();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
