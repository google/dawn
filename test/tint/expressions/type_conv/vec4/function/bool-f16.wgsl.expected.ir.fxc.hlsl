SKIP: FAILED


static bool t = false;
bool4 m() {
  t = true;
  return bool4((t).xxxx);
}

void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000001AD31221750(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

