SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool t = false;

bool m() {
  t = true;
  return bool(t);
}

void f() {
  const bool tint_symbol = m();
  float16_t v = float16_t(tint_symbol);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000001E8286B1350(15,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000001E8286B1350(15,13): error X3000: unrecognized identifier 'v'

