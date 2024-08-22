SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool t = false;

bool4 m() {
  t = true;
  return bool4((t).xxxx);
}

void f() {
  bool4 tint_symbol = m();
  vector<float16_t, 4> v = vector<float16_t, 4>(tint_symbol);
}
FXC validation failure:
C:\src\dawn\Shader@0x000001E475F50020(15,10-18): error X3000: syntax error: unexpected token 'float16_t'

