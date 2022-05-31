RWByteAddressBuffer sb_rw : register(u0, space0);

int tint_atomicAnd(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedAnd(offset, value, original_value);
  return original_value;
}


void atomicAnd_152966() {
  int res = tint_atomicAnd(sb_rw, 0u, 1);
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
