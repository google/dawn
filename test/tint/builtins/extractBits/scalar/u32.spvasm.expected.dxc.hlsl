uint tint_extract_bits(uint v, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint shl = (32u - e);
  const uint shr = (shl + s);
  const uint shl_result = ((shl < 32u) ? (v << shl) : 0u);
  return ((shr < 32u) ? (shl_result >> shr) : ((shl_result >> 31u) >> 1u));
}

void f_1() {
  uint v = 0u;
  uint offset_1 = 0u;
  uint count = 0u;
  const uint x_11 = tint_extract_bits(v, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
