int tint_insert_bits(int v, int n, uint offset, uint count) {
  const uint e = (offset + count);
  const uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0) & int(mask)) | (v & int(~(mask))));
}

void f_1() {
  int v = 0;
  int n = 0;
  uint offset_1 = 0u;
  uint count = 0u;
  const int x_17 = v;
  const int x_18 = n;
  const uint x_19 = offset_1;
  const uint x_20 = count;
  const int x_15 = tint_insert_bits(x_17, x_18, x_19, x_20);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
