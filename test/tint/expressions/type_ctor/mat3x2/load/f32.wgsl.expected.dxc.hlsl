[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float3x2 m = float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx);
  const float3x2 m_1 = float3x2(m);
}
