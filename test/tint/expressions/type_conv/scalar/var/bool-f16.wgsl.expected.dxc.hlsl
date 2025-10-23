
static bool u = true;
[numthreads(1, 1, 1)]
void f() {
  float16_t v = float16_t(u);
}

