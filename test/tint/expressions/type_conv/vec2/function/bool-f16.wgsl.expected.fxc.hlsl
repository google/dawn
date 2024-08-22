SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool t = false;

bool2 m() {
  t = true;
  return bool2((t).xx);
}

void f() {
  bool2 tint_symbol = m();
  vector<float16_t, 2> v = vector<float16_t, 2>(tint_symbol);
}
FXC validation failure:
C:\src\dawn\Shader@0x00000161EB49FF00(15,10-18): error X3000: syntax error: unexpected token 'float16_t'

