Texture2D<float4> x_2 : register(t0, space0);
SamplerState x_3 : register(s1, space0);

void main_1() {
  float4 var_1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_22 = x_2.Sample(x_3, float2(0.0f, 0.0f));
  const float4 x_26 = x_2.Sample(x_3, float2(0.0f, 0.0f));
  var_1 = (x_22 + x_26);
  return;
}

void main() {
  main_1();
  return;
}
