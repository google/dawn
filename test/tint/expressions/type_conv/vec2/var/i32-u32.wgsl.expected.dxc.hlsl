
static int2 u = (int(1)).xx;
[numthreads(1, 1, 1)]
void f() {
  uint2 v = uint2(u);
}

