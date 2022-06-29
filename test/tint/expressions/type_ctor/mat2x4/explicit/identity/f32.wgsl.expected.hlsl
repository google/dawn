[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float2x4 m = float2x4(float4(0.0f, 1.0f, 2.0f, 3.0f), float4(4.0f, 5.0f, 6.0f, 7.0f));

float2x4 f() {
  const float2x4 m_1 = float2x4(m);
  return m_1;
}
