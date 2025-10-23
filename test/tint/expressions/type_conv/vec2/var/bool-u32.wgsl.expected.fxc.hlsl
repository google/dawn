
static bool2 u = (true).xx;
[numthreads(1, 1, 1)]
void f() {
  uint2 v = uint2(u);
}

