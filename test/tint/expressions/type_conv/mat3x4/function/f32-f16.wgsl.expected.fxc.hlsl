SKIP: INVALID


static float t = 0.0f;
float3x4 m() {
  t = (t + 1.0f);
  return float3x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f));
}

void f() {
  matrix<float16_t, 3, 4> v = matrix<float16_t, 3, 4>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

