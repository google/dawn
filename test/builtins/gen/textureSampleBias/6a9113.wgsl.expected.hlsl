Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleBias_6a9113() {
  float4 res = arg_0.SampleBias(arg_1, float2(0.0f, 0.0f), 1.0f);
}

void fragment_main() {
  textureSampleBias_6a9113();
  return;
}
