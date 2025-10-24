
static int2 u = (int(1)).xx;
[numthreads(1, 1, 1)]
void f() {
  float2 v = float2(u);
}

