TextureCube arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

void textureSampleCompare_63fb83() {
  float res = arg_0.SampleCmp(arg_1, (1.0f).xxx, 1.0f);
}

void fragment_main() {
  textureSampleCompare_63fb83();
  return;
}
