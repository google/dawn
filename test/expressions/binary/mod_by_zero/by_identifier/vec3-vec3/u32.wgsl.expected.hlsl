[numthreads(1, 1, 1)]
void f() {
  uint3 a = uint3(1u, 2u, 3u);
  uint3 b = uint3(0u, 5u, 0u);
  const uint3 r = (a % (b == uint3(0u, 0u, 0u) ? uint3(1u, 1u, 1u) : b));
  return;
}
