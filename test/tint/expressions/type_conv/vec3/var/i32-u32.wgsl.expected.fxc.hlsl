
static int3 u = (int(1)).xxx;
[numthreads(1, 1, 1)]
void f() {
  uint3 v = uint3(u);
}

