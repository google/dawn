[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float2 m() {
  t = 1.0f;
  return float2((t).xx);
}

void f() {
  const float2 tint_symbol = m();
  int2 v = int2(tint_symbol);
}
