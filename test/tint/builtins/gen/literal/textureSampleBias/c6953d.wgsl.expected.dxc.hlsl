TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleBias_c6953d() {
  float4 res = arg_0.SampleBias(arg_1, float4((1.0f).xxx, float(1u)), 1.0f);
}

void fragment_main() {
  textureSampleBias_c6953d();
  return;
}
