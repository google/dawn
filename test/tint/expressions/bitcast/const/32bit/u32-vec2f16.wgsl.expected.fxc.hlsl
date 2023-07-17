SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  const vector<float16_t, 2> b = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x0000019F1F909150(3,16-24): error X3000: syntax error: unexpected token 'float16_t'

