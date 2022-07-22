[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float3x4 m = float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  const float3x4 m_1 = float3x4(m);
}
