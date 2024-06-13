Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float textureSampleCompare_dd431d() {
  float res = arg_0.SampleCmp(arg_1, float3((1.0f).xx, float(1)), 1.0f);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompare_dd431d()));
  return;
}
