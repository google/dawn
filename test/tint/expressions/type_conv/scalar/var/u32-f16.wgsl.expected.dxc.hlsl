
static uint u = 1u;
[numthreads(1, 1, 1)]
void f() {
  float16_t v = float16_t(u);
}

