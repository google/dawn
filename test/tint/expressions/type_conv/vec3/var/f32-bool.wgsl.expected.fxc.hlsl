
static float3 u = (1.0f).xxx;
[numthreads(1, 1, 1)]
void f() {
  bool3 v = bool3(u);
}

