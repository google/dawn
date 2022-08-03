SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  matrix<float16_t, 2, 3> m = matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
  const matrix<float16_t, 2, 3> m_1 = matrix<float16_t, 2, 3>(m);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x00000221FD786780(7,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x00000221FD786780(8,16-24): error X3000: syntax error: unexpected token 'float16_t'

