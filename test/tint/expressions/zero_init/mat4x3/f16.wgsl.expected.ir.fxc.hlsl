SKIP: FAILED


void f() {
  matrix<float16_t, 4, 3> v = matrix<float16_t, 4, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x00000284242D01E0(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

