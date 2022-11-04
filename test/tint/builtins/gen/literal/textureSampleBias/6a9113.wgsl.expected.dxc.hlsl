Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleBias_6a9113() {
  float4 res = arg_0.SampleBias(arg_1, (1.0f).xx, 1.0f);
}

void fragment_main() {
  textureSampleBias_6a9113();
  return;
}
