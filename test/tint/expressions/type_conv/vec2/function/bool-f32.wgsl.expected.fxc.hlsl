
static bool t = false;
bool2 m() {
  t = true;
  return bool2((t).xx);
}

[numthreads(1, 1, 1)]
void f() {
  float2 v = float2(m());
}

