
static uint4 u = (1u).xxxx;
[numthreads(1, 1, 1)]
void f() {
  int4 v = int4(u);
}

