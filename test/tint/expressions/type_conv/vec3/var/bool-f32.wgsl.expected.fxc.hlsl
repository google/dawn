
static bool3 u = (true).xxx;
[numthreads(1, 1, 1)]
void f() {
  float3 v = float3(u);
}

