[numthreads(1, 1, 1)]
void f() {
  const uint3 a = uint3(1u, 2u, 3u);
  const uint3 b = uint3(0u, 5u, 0u);
  const uint3 r = (a / uint3(1u, 5u, 1u));
  return;
}
