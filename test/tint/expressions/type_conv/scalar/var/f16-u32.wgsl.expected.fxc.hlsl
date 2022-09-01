SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float16_t u = float16_t(1.0h);

void f() {
  const uint v = uint(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000001FB41A62990(6,8-16): error X3000: unrecognized identifier 'float16_t'

