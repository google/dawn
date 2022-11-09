int3 tint_insert_bits(int3 v, int3 n, uint offset, uint count) {
  const uint e = (offset + count);
  const uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint3((offset).xxx)) : (0).xxx) & int3((int(mask)).xxx)) | (v & int3((int(~(mask))).xxx)));
}

void f_1() {
  int3 v = (0).xxx;
  int3 n = (0).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  const int3 x_18 = v;
  const int3 x_19 = n;
  const uint x_20 = offset_1;
  const uint x_21 = count;
  const int3 x_16 = tint_insert_bits(x_18, x_19, x_20, x_21);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
