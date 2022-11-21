[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer v : register(u0, space0);

int4 tint_mod(int4 lhs, int rhs) {
  const int4 r = int4((rhs).xxxx);
  return (lhs % (((r == (0).xxxx) | ((lhs == (-2147483648).xxxx) & (r == (-1).xxxx))) ? (1).xxxx : r));
}

void foo() {
  const int4 tint_symbol = tint_mod(asint(v.Load4(0u)), 2);
  v.Store4(0u, asuint(tint_symbol));
}
