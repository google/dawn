SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint t = 0u;

uint4 m() {
  t = 1u;
  return uint4((t).xxxx);
}

void f() {
  uint4 tint_symbol = m();
  vector<float16_t, 4> v = vector<float16_t, 4>(tint_symbol);
}
FXC validation failure:
C:\src\dawn\Shader@0x00000242E9073AF0(15,10-18): error X3000: syntax error: unexpected token 'float16_t'

