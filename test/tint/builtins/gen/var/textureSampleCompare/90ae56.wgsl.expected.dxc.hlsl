Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleCompare_90ae56() {
  float2 arg_2 = (1.0f).xx;
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  float res = arg_0.SampleCmp(arg_1, float3(arg_2, float(arg_3)), arg_4);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  textureSampleCompare_90ae56();
  return;
}
