[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float4x4 m = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  const float4x4 m_1 = float4x4(m);
}
