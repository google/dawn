[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float3x2 m = float3x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f), float2(4.0f, 5.0f));

float3x2 f() {
  const float3x2 m_1 = float3x2(m);
  return m_1;
}
