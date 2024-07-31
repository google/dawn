SKIP: FAILED


void f() {
  matrix<float16_t, 3, 4> v = matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x000001668305E720(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

