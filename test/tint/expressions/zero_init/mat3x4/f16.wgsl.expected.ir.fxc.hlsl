SKIP: INVALID


void f() {
  matrix<float16_t, 3, 4> v = matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000002288DB61FD0(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

