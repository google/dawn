uint tint_insert_bits(uint v, uint n, uint offset, uint count) {
  const uint e = (offset + count);
  const uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0u) & mask) | (v & ~(mask)));
}

void f_1() {
  uint v = 0u;
  uint n = 0u;
  uint offset_1 = 0u;
  uint count = 0u;
  const uint x_14 = v;
  const uint x_15 = n;
  const uint x_16 = offset_1;
  const uint x_17 = count;
  const uint x_12 = tint_insert_bits(x_14, x_15, x_16, x_17);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
