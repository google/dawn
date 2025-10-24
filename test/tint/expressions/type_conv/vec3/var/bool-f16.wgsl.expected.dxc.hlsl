
static bool3 u = (true).xxx;
[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(u);
}

