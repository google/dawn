SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool2 u = (true).xx;

void f() {
  const vector<float16_t, 2> v = vector<float16_t, 2>(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000002B8624A4D70(9,16-24): error X3000: syntax error: unexpected token 'float16_t'

