
static float t = 0.0f;
float2 m() {
  t = 1.0f;
  return float2((t).xx);
}

[numthreads(1, 1, 1)]
void f() {
  bool2 v = bool2(m());
}

