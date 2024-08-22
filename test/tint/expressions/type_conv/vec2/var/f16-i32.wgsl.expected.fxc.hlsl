SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static vector<float16_t, 2> u = (float16_t(1.0h)).xx;

void f() {
  int2 v = int2(u);
}
FXC validation failure:
C:\src\dawn\Shader@0x000001747A490930(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

