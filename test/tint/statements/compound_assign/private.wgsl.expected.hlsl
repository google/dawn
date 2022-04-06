[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int a = 0;
static float4 b = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float2x2 c = float2x2(0.0f, 0.0f, 0.0f, 0.0f);

void foo() {
  a = (a / 2);
  b = mul(float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), b);
  c = (c * 2.0f);
}
