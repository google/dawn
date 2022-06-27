[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float2x3 m = float2x3((0.0f).xxx, (0.0f).xxx);
  const float2x3 m_1 = float2x3(m);
}
