SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int4 u = (1).xxxx;

void f() {
  const vector<float16_t, 4> v = vector<float16_t, 4>(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x0000020AD0F04D70(9,16-24): error X3000: syntax error: unexpected token 'float16_t'

