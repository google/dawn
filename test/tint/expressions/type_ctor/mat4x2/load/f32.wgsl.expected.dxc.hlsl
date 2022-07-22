[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float4x2 m = float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx);
  const float4x2 m_1 = float4x2(m);
}
