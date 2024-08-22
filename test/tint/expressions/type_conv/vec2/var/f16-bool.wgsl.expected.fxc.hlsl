SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static vector<float16_t, 2> u = (float16_t(1.0h)).xx;

void f() {
  bool2 v = bool2(u);
}
FXC validation failure:
C:\src\dawn\Shader@0x000002731BDF77E0(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

