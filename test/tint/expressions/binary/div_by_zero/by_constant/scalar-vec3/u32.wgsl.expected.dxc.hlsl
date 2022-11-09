uint3 tint_div(uint lhs, uint3 rhs) {
  const uint3 l = uint3((lhs).xxx);
  return (l / ((rhs == (0u).xxx) ? (1u).xxx : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  const uint a = 4u;
  const uint3 b = uint3(0u, 2u, 0u);
  const uint3 r = tint_div(a, b);
  return;
}
