Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

void textureSampleCompare_dec064() {
  float res = arg_0.SampleCmp(arg_1, (0.0f).xx, 1.0f, (0).xx);
}

void fragment_main() {
  textureSampleCompare_dec064();
  return;
}
