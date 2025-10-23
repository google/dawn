
static bool u = true;
[numthreads(1, 1, 1)]
void f() {
  int v = int(u);
}

