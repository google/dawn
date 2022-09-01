SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float16_t t = float16_t(0.0h);

float16_t m() {
  t = float16_t(1.0h);
  return float16_t(t);
}

void f() {
  const float16_t tint_symbol = m();
  uint v = uint(tint_symbol);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000001DAF6968AE0(6,8-16): error X3000: unrecognized identifier 'float16_t'

