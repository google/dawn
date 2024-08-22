SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float2x2 m() {
  t = (t + 1.0f);
  return float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
}

void f() {
  float2x2 tint_symbol = m();
  matrix<float16_t, 2, 2> v = matrix<float16_t, 2, 2>(tint_symbol);
}
FXC validation failure:
C:\src\dawn\Shader@0x0000027B6804FFE0(15,10-18): error X3000: syntax error: unexpected token 'float16_t'

