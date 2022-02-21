[numthreads(1, 1, 1)]
void f() {
  const uint3 a = uint3(1u, 2u, 3u);
  const float3 b = asfloat(a);
  return;
}
