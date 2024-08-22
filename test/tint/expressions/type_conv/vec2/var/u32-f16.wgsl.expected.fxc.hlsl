SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint2 u = (1u).xx;

void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(u);
}
FXC validation failure:
C:\src\dawn\Shader@0x000002104E5D5420(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

