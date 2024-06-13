Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float textureSampleCompare_dec064() {
  float res = arg_0.SampleCmp(arg_1, (1.0f).xx, 1.0f, int2((1).xx));
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompare_dec064()));
  return;
}
