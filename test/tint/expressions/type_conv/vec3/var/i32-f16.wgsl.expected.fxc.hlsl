SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int3 u = (1).xxx;

void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(u);
}
FXC validation failure:
C:\src\dawn\Shader@0x000002AF74EE3280(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

