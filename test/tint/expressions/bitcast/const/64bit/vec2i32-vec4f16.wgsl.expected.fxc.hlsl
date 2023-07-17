SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  const vector<float16_t, 4> b = vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(-4.0h));
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x0000020858736DC0(3,16-24): error X3000: syntax error: unexpected token 'float16_t'

