uint3 tint_extract_bits(uint3 v, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint shl = (32u - e);
  const uint shr = (shl + s);
  return ((v << uint3((shl).xxx)) >> uint3((shr).xxx));
}

void f_1() {
  uint3 v = uint3(0u, 0u, 0u);
  uint offset_1 = 0u;
  uint count = 0u;
  const uint3 x_14 = tint_extract_bits(v, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
