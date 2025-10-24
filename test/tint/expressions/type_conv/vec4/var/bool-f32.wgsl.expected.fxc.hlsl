
static bool4 u = (true).xxxx;
[numthreads(1, 1, 1)]
void f() {
  float4 v = float4(u);
}

