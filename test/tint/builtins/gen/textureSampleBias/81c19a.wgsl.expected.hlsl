Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleBias_81c19a() {
  float4 res = arg_0.SampleBias(arg_1, float2(0.0f, 0.0f), 1.0f, int2(0, 0));
}

void fragment_main() {
  textureSampleBias_81c19a();
  return;
}
