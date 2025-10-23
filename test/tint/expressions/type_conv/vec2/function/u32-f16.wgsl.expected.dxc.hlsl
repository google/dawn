
static uint t = 0u;
uint2 m() {
  t = 1u;
  return uint2((t).xx);
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(m());
}

