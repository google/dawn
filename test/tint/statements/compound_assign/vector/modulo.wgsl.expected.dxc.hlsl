[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer v : register(u0, space0);

int4 tint_mod(int4 lhs, int4 rhs) {
  return (lhs % (((rhs == (0).xxxx) | ((lhs == (-2147483648).xxxx) & (rhs == (-1).xxxx))) ? (1).xxxx : rhs));
}

void foo() {
  const int4 tint_symbol = tint_mod(asint(v.Load4(0u)), (2).xxxx);
  v.Store4(0u, asuint(tint_symbol));
}
