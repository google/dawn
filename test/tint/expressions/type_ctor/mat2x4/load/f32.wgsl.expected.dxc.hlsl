[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float2x4 m = float2x4((0.0f).xxxx, (0.0f).xxxx);
  const float2x4 m_1 = float2x4(m);
}
