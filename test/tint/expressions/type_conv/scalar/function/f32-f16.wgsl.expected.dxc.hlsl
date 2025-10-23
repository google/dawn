
static float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}

[numthreads(1, 1, 1)]
void f() {
  float16_t v = float16_t(m());
}

