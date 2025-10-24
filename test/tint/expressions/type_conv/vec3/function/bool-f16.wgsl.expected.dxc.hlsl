
static bool t = false;
bool3 m() {
  t = true;
  return bool3((t).xxx);
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(m());
}

