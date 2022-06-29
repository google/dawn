[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float4x2 m = float4x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f), float2(4.0f, 5.0f), float2(6.0f, 7.0f));

float4x2 f() {
  const float4x2 m_1 = float4x2(m);
  return m_1;
}
