
static float2 u = (1.0f).xx;
[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(u);
}

