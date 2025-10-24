
static float4 u = (1.0f).xxxx;
[numthreads(1, 1, 1)]
void f() {
  bool4 v = bool4(u);
}

