
static bool t = false;
bool m() {
  t = true;
  return bool(t);
}

[numthreads(1, 1, 1)]
void f() {
  float v = float(m());
}

