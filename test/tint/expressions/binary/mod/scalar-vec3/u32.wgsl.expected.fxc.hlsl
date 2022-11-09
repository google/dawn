uint3 tint_mod(uint lhs, uint3 rhs) {
  const uint3 l = uint3((lhs).xxx);
  return (l % ((rhs == (0u).xxx) ? (1u).xxx : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  const uint a = 4u;
  const uint3 b = uint3(1u, 2u, 3u);
  const uint3 r = tint_mod(a, b);
  return;
}
