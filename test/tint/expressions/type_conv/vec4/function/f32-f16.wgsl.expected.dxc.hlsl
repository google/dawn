
static float t = 0.0f;
float4 m() {
  t = 1.0f;
  return float4((t).xxxx);
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(m());
}

