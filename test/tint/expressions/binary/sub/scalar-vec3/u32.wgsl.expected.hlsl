[numthreads(1, 1, 1)]
void f() {
  const uint a = 4u;
  const uint3 b = uint3(1u, 2u, 3u);
  const uint3 r = (4u - uint3(1u, 2u, 3u));
  return;
}
