
static uint2 u = (1u).xx;
[numthreads(1, 1, 1)]
void f() {
  int2 v = int2(u);
}

