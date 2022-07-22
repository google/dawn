RWByteAddressBuffer sb_rw : register(u0, space0);

int tint_atomicLoad(RWByteAddressBuffer buffer, uint offset) {
  int value = 0;
  buffer.InterlockedOr(offset, 0, value);
  return value;
}


void atomicLoad_0806ad() {
  int res = tint_atomicLoad(sb_rw, 0u);
}

void fragment_main() {
  atomicLoad_0806ad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicLoad_0806ad();
  return;
}
