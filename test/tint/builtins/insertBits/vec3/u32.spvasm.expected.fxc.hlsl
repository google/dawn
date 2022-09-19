uint3 tint_insert_bits(uint3 v, uint3 n, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint mask = (((1u << s) - 1u) ^ ((1u << e) - 1u));
  return (((n << uint3((s).xxx)) & uint3((mask).xxx)) | (v & uint3((~(mask)).xxx)));
}

void f_1() {
  uint3 v = (0u).xxx;
  uint3 n = (0u).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  const uint3 x_17 = v;
  const uint3 x_18 = n;
  const uint x_19 = offset_1;
  const uint x_20 = count;
  const uint3 x_15 = tint_insert_bits(x_17, x_18, x_19, x_20);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
