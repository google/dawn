
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float textureSampleCompare_1912e5() {
  float3 arg_2 = (1.0f).xxx;
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  TextureCubeArray v = arg_0;
  SamplerComparisonState v_1 = arg_1;
  float3 v_2 = arg_2;
  float v_3 = arg_4;
  float res = v.SampleCmp(v_1, float4(v_2, float(arg_3)), v_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompare_1912e5()));
}

