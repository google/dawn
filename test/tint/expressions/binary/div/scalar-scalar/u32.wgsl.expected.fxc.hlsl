uint tint_div(uint lhs, uint rhs) {
  return (lhs / ((rhs == 0u) ? 1u : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  const uint a = 1u;
  const uint b = 2u;
  const uint r = tint_div(a, b);
  return;
}
