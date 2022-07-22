[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void foo() {
  int a = 0;
  float4 b = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 c = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  a = (a / 2);
  b = mul(float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx), b);
  c = (c * 2.0f);
}
