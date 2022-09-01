SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint t = 0u;

uint m() {
  t = 1u;
  return uint(t);
}

void f() {
  const uint tint_symbol = m();
  float16_t v = float16_t(tint_symbol);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x0000027A60011350(15,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x0000027A60011350(15,13): error X3000: unrecognized identifier 'v'

