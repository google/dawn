SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int u = 1;

void f() {
  const float16_t v = float16_t(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x0000026C24F72E80(9,9-17): error X3000: unrecognized identifier 'float16_t'

