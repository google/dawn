SKIP: FAILED


void f() {
  matrix<float16_t, 4, 2> v = matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x000001CC16AC0270(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

