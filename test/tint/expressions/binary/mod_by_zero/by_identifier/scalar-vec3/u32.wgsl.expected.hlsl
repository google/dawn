[numthreads(1, 1, 1)]
void f() {
  uint a = 4u;
  uint3 b = uint3(0u, 2u, 0u);
  const uint3 r = (a % (b == uint3(0u, 0u, 0u) ? uint3(1u, 1u, 1u) : b));
  return;
}
