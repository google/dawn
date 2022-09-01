SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static vector<float16_t, 2> u = (float16_t(1.0h)).xx;

void f() {
  const float2 v = float2(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x00000163B9F34D70(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

