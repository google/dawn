
static vector<float16_t, 2> u = (float16_t(1.0h)).xx;
[numthreads(1, 1, 1)]
void f() {
  float2 v = float2(u);
}

