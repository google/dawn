
static bool u = true;
[numthreads(1, 1, 1)]
void f() {
  float v = float(u);
}

