
static bool t = false;
bool4 m() {
  t = true;
  return bool4((t).xxxx);
}

[numthreads(1, 1, 1)]
void f() {
  float4 v = float4(m());
}

