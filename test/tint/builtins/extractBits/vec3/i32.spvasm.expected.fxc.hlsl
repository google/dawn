int3 tint_extract_bits(int3 v, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint shl = (32u - e);
  const uint shr = (shl + s);
  return ((v << uint3((shl).xxx)) >> uint3((shr).xxx));
}

void f_1() {
  int3 v = (0).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  const int3 x_17 = v;
  const uint x_18 = offset_1;
  const uint x_19 = count;
  const int3 x_15 = tint_extract_bits(x_17, x_18, x_19);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
