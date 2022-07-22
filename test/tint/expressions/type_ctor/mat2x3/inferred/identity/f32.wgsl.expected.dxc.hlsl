[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float2x3 m = float2x3(float3(0.0f, 1.0f, 2.0f), float3(3.0f, 4.0f, 5.0f));

float2x3 f() {
  const float2x3 m_1 = float2x3(m);
  return m_1;
}
