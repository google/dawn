SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  const matrix<float16_t, 3, 3> a = matrix<float16_t, 3, 3>(vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 3>(float16_t(4.0h), float16_t(5.0h), float16_t(6.0h)), vector<float16_t, 3>(float16_t(7.0h), float16_t(8.0h), float16_t(9.0h)));
  const matrix<float16_t, 3, 3> b = matrix<float16_t, 3, 3>(vector<float16_t, 3>(float16_t(-1.0h), float16_t(-2.0h), float16_t(-3.0h)), vector<float16_t, 3>(float16_t(-4.0h), float16_t(-5.0h), float16_t(-6.0h)), vector<float16_t, 3>(float16_t(-7.0h), float16_t(-8.0h), float16_t(-9.0h)));
  const matrix<float16_t, 3, 3> r = (a - b);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x0000014C51995C90(3,16-24): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x0000014C51995C90(4,16-24): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x0000014C51995C90(5,16-24): error X3000: syntax error: unexpected token 'float16_t'

