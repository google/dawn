
static float4x2 u = float4x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f), float2(7.0f, 8.0f));
void f() {
  matrix<float16_t, 4, 2> v = matrix<float16_t, 4, 2>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

