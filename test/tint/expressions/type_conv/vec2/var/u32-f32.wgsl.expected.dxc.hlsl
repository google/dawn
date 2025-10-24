
static uint2 u = (1u).xx;
[numthreads(1, 1, 1)]
void f() {
  float2 v = float2(u);
}

