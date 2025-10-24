
static int4 u = (int(1)).xxxx;
[numthreads(1, 1, 1)]
void f() {
  bool4 v = bool4(u);
}

