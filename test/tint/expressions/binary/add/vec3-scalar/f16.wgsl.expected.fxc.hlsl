SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  const vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  const vector<float16_t, 3> r = (a + float16_t(4.0h));
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x0000023450E536E0(3,16-24): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x0000023450E536E0(4,16-24): error X3000: syntax error: unexpected token 'float16_t'

