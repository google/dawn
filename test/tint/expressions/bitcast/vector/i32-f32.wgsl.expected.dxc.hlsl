[numthreads(1, 1, 1)]
void f() {
  const int3 a = int3(1, 2, 3);
  const float3 b = asfloat(a);
  return;
}
