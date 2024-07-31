SKIP: FAILED


void f() {
  matrix<float16_t, 3, 2> v = matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x000001CC2E90DC70(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

