SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  const vector<float16_t, 4> a = vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(-4.0h));
  const vector<float16_t, 4> b = a;
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x0000021AD80372B0(3,16-24): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x0000021AD80372B0(4,16-24): error X3000: syntax error: unexpected token 'float16_t'

