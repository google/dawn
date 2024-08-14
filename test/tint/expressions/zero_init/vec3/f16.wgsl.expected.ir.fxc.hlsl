SKIP: FAILED


void f() {
  vector<float16_t, 3> v = (float16_t(0.0h)).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000001DF51E9DF50(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

