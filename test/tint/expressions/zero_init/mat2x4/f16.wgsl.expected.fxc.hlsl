SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  matrix<float16_t, 2, 4> v = matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x00000216FF5427E0(7,10-18): error X3000: syntax error: unexpected token 'float16_t'

