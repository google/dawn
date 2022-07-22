[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float2x2 m = float2x2((0.0f).xx, (0.0f).xx);
  const float2x2 m_1 = float2x2(m);
}
