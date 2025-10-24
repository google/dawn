
static int2 u = (int(1)).xx;
[numthreads(1, 1, 1)]
void f() {
  bool2 v = bool2(u);
}

