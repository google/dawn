
static bool2 u = (true).xx;
[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(u);
}

