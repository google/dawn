SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  matrix<float16_t, 3, 3> m = matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
  const matrix<float16_t, 3, 3> m_1 = matrix<float16_t, 3, 3>(m);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001DD9A7165C0(7,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001DD9A7165C0(8,16-24): error X3000: syntax error: unexpected token 'float16_t'

