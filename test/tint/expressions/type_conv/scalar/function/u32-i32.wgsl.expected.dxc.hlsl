
static uint t = 0u;
uint m() {
  t = 1u;
  return uint(t);
}

[numthreads(1, 1, 1)]
void f() {
  int v = int(m());
}

