SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint u = 1u;

void f() {
  const float16_t v = float16_t(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000002C39A2D2A10(9,9-17): error X3000: unrecognized identifier 'float16_t'

