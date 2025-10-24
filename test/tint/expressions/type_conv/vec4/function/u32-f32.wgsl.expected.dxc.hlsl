
static uint t = 0u;
uint4 m() {
  t = 1u;
  return uint4((t).xxxx);
}

[numthreads(1, 1, 1)]
void f() {
  float4 v = float4(m());
}

