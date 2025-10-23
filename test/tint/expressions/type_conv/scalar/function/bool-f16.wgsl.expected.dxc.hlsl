
static bool t = false;
bool m() {
  t = true;
  return bool(t);
}

[numthreads(1, 1, 1)]
void f() {
  float16_t v = float16_t(m());
}

