SKIP: INVALID


static int t = 0;
int2 m() {
  t = 1;
  return int2((t).xx);
}

void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000002379AF09150(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

