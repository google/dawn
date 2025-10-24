
static uint t = 0u;
uint4 m() {
  t = 1u;
  return uint4((t).xxxx);
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(m());
}

