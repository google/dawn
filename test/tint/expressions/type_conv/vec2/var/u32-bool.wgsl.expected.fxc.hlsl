
static uint2 u = (1u).xx;
[numthreads(1, 1, 1)]
void f() {
  bool2 v = bool2(u);
}

