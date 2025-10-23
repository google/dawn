
static uint t = 0u;
uint2 m() {
  t = 1u;
  return uint2((t).xx);
}

[numthreads(1, 1, 1)]
void f() {
  float2 v = float2(m());
}

