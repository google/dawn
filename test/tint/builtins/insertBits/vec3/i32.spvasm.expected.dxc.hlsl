int3 tint_insert_bits(int3 v, int3 n, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint mask = (((1u << s) - 1u) ^ ((1u << e) - 1u));
  return (((n << uint3((s).xxx)) & int3((int(mask)).xxx)) | (v & int3((int(~(mask))).xxx)));
}

void f_1() {
  int3 v = (0).xxx;
  int3 n = (0).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  const int3 x_16 = tint_insert_bits(v, n, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
