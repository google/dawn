
static bool u = true;
[numthreads(1, 1, 1)]
void f() {
  uint v = uint(u);
}

