
static uint u = 1u;
[numthreads(1, 1, 1)]
void f() {
  bool v = bool(u);
}

