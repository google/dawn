
static bool t = false;
bool4 m() {
  t = true;
  return bool4((t).xxxx);
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(m());
}

