SamplerComparisonState x_10 : register(s0, space0);
Texture2D x_20 : register(t1, space0);

void main_1() {
  const float x_131 = x_20.SampleCmp(x_10, float2(0.0f, 0.0f), 0.200000003f);
  return;
}

void main() {
  main_1();
  return;
}
