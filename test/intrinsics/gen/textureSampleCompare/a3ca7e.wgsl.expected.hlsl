TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

void textureSampleCompare_a3ca7e() {
  float res = arg_0.SampleCmp(arg_1, float4(0.0f, 0.0f, 0.0f, float(1)), 1.0f);
}

void fragment_main() {
  textureSampleCompare_a3ca7e();
  return;
}
