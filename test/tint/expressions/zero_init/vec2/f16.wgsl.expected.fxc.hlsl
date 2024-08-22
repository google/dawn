SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  vector<float16_t, 2> v = (float16_t(0.0h)).xx;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001780C680B30(7,10-18): error X3000: syntax error: unexpected token 'float16_t'

