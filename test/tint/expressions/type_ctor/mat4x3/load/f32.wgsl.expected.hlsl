[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float4x3 m = float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  const float4x3 m_1 = float4x3(m);
}
