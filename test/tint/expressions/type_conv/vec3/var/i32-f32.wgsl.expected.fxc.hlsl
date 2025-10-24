
static int3 u = (int(1)).xxx;
[numthreads(1, 1, 1)]
void f() {
  float3 v = float3(u);
}

