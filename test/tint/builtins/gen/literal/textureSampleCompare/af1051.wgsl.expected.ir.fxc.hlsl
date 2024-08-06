
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float textureSampleCompare_af1051() {
  Texture2DArray v = arg_0;
  SamplerComparisonState v_1 = arg_1;
  float res = v.SampleCmp(v_1, float3((1.0f).xx, float(1)), 1.0f, (1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompare_af1051()));
}

