
static uint4 u = (1u).xxxx;
[numthreads(1, 1, 1)]
void f() {
  bool4 v = bool4(u);
}

