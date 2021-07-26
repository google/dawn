[numthreads(1, 1, 1)]
void f() {
  const int3 a = int3(1, 2, 3);
  const uint3 b = asuint(a);
  return;
}
