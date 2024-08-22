SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static vector<float16_t, 3> u = (float16_t(1.0h)).xxx;

void f() {
  uint3 v = uint3(u);
}
FXC validation failure:
C:\src\dawn\Shader@0x00000220F8EF87A0(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

