
static int4 u = (int(1)).xxxx;
[numthreads(1, 1, 1)]
void f() {
  uint4 v = uint4(u);
}

