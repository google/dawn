
static int3 u = (int(1)).xxx;
[numthreads(1, 1, 1)]
void f() {
  bool3 v = bool3(u);
}

