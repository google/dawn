uint tint_extract_bits(uint v, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint shl = (32u - e);
  const uint shr = (shl + s);
  return ((v << shl) >> shr);
}

void f_1() {
  uint v = 0u;
  uint offset_1 = 0u;
  uint count = 0u;
  const uint x_13 = v;
  const uint x_14 = offset_1;
  const uint x_15 = count;
  const uint x_11 = tint_extract_bits(x_13, x_14, x_15);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
