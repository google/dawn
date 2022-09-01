[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float3 m() {
  t = 1.0f;
  return float3((t).xxx);
}

void f() {
  const float3 tint_symbol = m();
  uint3 v = uint3(tint_symbol);
}
