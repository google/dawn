SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint4 u = (1u).xxxx;

void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(u);
}
FXC validation failure:
C:\src\dawn\Shader@0x000001BCF14E0930(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

