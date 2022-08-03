SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  const vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  const vector<float16_t, 3> b = vector<float16_t, 3>(float16_t(0.0h), float16_t(5.0h), float16_t(0.0h));
  const vector<float16_t, 3> r = (a / b);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000002160BCD37C0(3,16-24): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000002160BCD37C0(4,16-24): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000002160BCD37C0(5,16-24): error X3000: syntax error: unexpected token 'float16_t'

