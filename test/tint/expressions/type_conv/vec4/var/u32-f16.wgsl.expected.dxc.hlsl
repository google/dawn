
static uint4 u = (1u).xxxx;
[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(u);
}

