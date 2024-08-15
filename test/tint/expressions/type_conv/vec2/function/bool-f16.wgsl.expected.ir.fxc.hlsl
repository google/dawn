SKIP: INVALID


static bool t = false;
bool2 m() {
  t = true;
  return bool2((t).xx);
}

void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000002282F921AC0(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

