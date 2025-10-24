
static uint t = 0u;
uint3 m() {
  t = 1u;
  return uint3((t).xxx);
}

[numthreads(1, 1, 1)]
void f() {
  bool3 v = bool3(m());
}

