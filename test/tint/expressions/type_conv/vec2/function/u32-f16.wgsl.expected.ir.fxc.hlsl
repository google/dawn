SKIP: FAILED


static uint t = 0u;
uint2 m() {
  t = 1u;
  return uint2((t).xx);
}

void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000001B32F3FDE20(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

