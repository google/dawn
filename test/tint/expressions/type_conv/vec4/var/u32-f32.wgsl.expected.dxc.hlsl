
static uint4 u = (1u).xxxx;
[numthreads(1, 1, 1)]
void f() {
  float4 v = float4(u);
}

