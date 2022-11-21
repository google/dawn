[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer v : register(u0, space0);

int tint_div(int lhs, int rhs) {
  return (lhs / (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs));
}

void foo() {
  const int tint_symbol = tint_div(asint(v.Load(0u)), 2);
  v.Store(0u, asuint(tint_symbol));
}
