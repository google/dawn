SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  matrix<float16_t, 2, 4> m = matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
  const matrix<float16_t, 2, 4> m_1 = matrix<float16_t, 2, 4>(m);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001DD62E069C0(7,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001DD62E069C0(8,16-24): error X3000: syntax error: unexpected token 'float16_t'

