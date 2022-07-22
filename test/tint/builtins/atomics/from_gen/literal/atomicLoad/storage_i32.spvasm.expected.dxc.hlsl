RWByteAddressBuffer sb_rw : register(u0, space0);

int tint_atomicLoad(RWByteAddressBuffer buffer, uint offset) {
  int value = 0;
  buffer.InterlockedOr(offset, 0, value);
  return value;
}


void atomicLoad_0806ad() {
  int res = 0;
  const int x_9 = tint_atomicLoad(sb_rw, 0u);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicLoad_0806ad();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicLoad_0806ad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
