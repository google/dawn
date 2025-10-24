
static bool t = false;
bool3 m() {
  t = true;
  return bool3((t).xxx);
}

[numthreads(1, 1, 1)]
void f() {
  float3 v = float3(m());
}

