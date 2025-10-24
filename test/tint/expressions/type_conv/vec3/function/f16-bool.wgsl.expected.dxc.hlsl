
static float16_t t = float16_t(0.0h);
vector<float16_t, 3> m() {
  t = float16_t(1.0h);
  return vector<float16_t, 3>((t).xxx);
}

[numthreads(1, 1, 1)]
void f() {
  bool3 v = bool3(m());
}

