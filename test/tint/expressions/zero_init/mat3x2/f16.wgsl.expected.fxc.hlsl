SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  matrix<float16_t, 3, 2> v = matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
}
FXC validation failure:
C:\src\dawn\Shader@0x000001D1924F2E20(7,10-18): error X3000: syntax error: unexpected token 'float16_t'

