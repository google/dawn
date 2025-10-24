
static int t = int(0);
int3 m() {
  t = int(1);
  return int3((t).xxx);
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(m());
}

