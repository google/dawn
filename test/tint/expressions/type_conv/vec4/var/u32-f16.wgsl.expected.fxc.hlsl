SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint4 u = (1u).xxxx;

void f() {
  const vector<float16_t, 4> v = vector<float16_t, 4>(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x0000020D1A4520E0(9,16-24): error X3000: syntax error: unexpected token 'float16_t'

