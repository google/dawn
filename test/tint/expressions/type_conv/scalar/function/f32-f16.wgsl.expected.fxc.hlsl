SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float m() {
  t = 1.0f;
  return float(t);
}

void f() {
  const float tint_symbol = m();
  float16_t v = float16_t(tint_symbol);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000002636E171350(15,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000002636E171350(15,13): error X3000: unrecognized identifier 'v'

