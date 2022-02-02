Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

void textureSampleCompare_3a5923() {
  float res = arg_0.SampleCmp(arg_1, float2(0.0f, 0.0f), 1.0f);
}

void fragment_main() {
  textureSampleCompare_3a5923();
  return;
}
