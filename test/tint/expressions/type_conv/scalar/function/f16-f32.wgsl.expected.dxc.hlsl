
static float16_t t = float16_t(0.0h);
float16_t m() {
  t = float16_t(1.0h);
  return float16_t(t);
}

[numthreads(1, 1, 1)]
void f() {
  float v = float(m());
}

