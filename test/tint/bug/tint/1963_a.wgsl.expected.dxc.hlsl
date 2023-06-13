[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void X(float2 a, float2 b) {
}

float2 Y() {
  return (0.0f).xx;
}

void f() {
  float2 v = float2(0.0f, 0.0f);
  X((0.0f).xx, v);
  const float2 tint_symbol = (0.0f).xx;
  const float2 tint_symbol_1 = Y();
  X(tint_symbol, tint_symbol_1);
}
