[numthreads(1, 1, 1)]
void f() {
  const uint3 a = uint3(1u, 2u, 3u);
  const uint3 r = (a / 4u);
  return;
}
