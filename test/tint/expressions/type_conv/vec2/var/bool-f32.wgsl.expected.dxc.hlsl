
static bool2 u = (true).xx;
[numthreads(1, 1, 1)]
void f() {
  float2 v = float2(u);
}

