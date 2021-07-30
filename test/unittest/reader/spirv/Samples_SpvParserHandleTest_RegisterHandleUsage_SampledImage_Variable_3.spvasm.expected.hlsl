SamplerState x_10 : register(s0, space0);
Texture2D<float4> x_20 : register(t1, space0);

void main_1() {
  const float4 x_131 = x_20.SampleLevel(x_10, float2(0.0f, 0.0f), 0.0f);
  return;
}

void main() {
  main_1();
  return;
}
