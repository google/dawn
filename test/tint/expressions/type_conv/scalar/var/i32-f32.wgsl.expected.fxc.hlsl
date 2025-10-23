
static int u = int(1);
[numthreads(1, 1, 1)]
void f() {
  float v = float(u);
}

