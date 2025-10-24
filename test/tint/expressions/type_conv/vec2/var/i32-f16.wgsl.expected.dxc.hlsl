
static int2 u = (int(1)).xx;
[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(u);
}

