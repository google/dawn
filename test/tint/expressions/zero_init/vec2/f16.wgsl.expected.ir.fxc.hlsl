SKIP: INVALID


void f() {
  vector<float16_t, 2> v = (float16_t(0.0h)).xx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000001588F010B40(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

