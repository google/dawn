[numthreads(1, 1, 1)]
void f() {
  const uint a = 4u;
  const uint3 b = uint3(0u, 2u, 0u);
  const uint3 r = (a / uint3(1u, 2u, 1u));
  return;
}
