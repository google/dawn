TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleCompare_a3ca7e() {
  float res = arg_0.SampleCmp(arg_1, float4((1.0f).xxx, float(1)), 1.0f);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  textureSampleCompare_a3ca7e();
  return;
}
