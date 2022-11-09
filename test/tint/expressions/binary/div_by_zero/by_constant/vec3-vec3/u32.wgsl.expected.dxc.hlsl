uint3 tint_div(uint3 lhs, uint3 rhs) {
  return (lhs / ((rhs == (0u).xxx) ? (1u).xxx : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  const uint3 a = uint3(1u, 2u, 3u);
  const uint3 b = uint3(0u, 5u, 0u);
  const uint3 r = tint_div(a, b);
  return;
}
