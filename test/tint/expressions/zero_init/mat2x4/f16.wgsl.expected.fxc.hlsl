SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  matrix<float16_t, 2, 4> v = matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
}
FXC validation failure:
C:\src\dawn\Shader@0x000001FDC6CFFE70(7,10-18): error X3000: syntax error: unexpected token 'float16_t'

