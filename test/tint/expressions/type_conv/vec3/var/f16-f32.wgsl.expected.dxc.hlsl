
static vector<float16_t, 3> u = (float16_t(1.0h)).xxx;
[numthreads(1, 1, 1)]
void f() {
  float3 v = float3(u);
}

