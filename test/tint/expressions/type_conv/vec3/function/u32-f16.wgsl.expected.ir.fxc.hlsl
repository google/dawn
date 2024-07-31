SKIP: FAILED


static uint t = 0u;
uint3 m() {
  t = 1u;
  return uint3((t).xxx);
}

void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x000002955500E390(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

