
static bool t = false;
bool m() {
  t = true;
  return bool(t);
}

[numthreads(1, 1, 1)]
void f() {
  uint v = uint(m());
}

