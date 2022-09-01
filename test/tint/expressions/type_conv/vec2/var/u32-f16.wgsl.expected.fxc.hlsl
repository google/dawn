SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint2 u = (1u).xx;

void f() {
  const vector<float16_t, 2> v = vector<float16_t, 2>(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x00000242F4738800(9,16-24): error X3000: syntax error: unexpected token 'float16_t'

