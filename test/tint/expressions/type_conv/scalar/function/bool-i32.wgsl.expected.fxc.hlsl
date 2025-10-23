
static bool t = false;
bool m() {
  t = true;
  return bool(t);
}

[numthreads(1, 1, 1)]
void f() {
  int v = int(m());
}

