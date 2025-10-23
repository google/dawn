
static int u = int(1);
[numthreads(1, 1, 1)]
void f() {
  float16_t v = float16_t(u);
}

