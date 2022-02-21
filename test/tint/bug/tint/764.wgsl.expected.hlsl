[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  const float4x4 m = float4x4(float4(1.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f));
  const float4 v1 = m[0];
  const float a = v1[0];
}
