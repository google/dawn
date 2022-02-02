Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

void textureSampleCompare_98b85c() {
  float res = arg_0.SampleCmp(arg_1, float3(0.0f, 0.0f, float(1)), 1.0f, int2(0, 0));
}

void fragment_main() {
  textureSampleCompare_98b85c();
  return;
}
