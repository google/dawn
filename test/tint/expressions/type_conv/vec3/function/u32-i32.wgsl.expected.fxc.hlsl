
static uint t = 0u;
uint3 m() {
  t = 1u;
  return uint3((t).xxx);
}

[numthreads(1, 1, 1)]
void f() {
  int3 v = int3(m());
}

