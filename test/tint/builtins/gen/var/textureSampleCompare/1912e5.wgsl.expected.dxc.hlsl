TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

void textureSampleCompare_1912e5() {
  float3 arg_2 = (0.0f).xxx;
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  float res = arg_0.SampleCmp(arg_1, float4(arg_2, float(arg_3)), arg_4);
}

void fragment_main() {
  textureSampleCompare_1912e5();
  return;
}
