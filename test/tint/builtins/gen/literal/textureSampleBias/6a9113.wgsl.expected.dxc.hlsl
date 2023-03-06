Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleBias_6a9113() {
  float4 res = arg_0.SampleBias(arg_1, (1.0f).xx, 1.0f);
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  textureSampleBias_6a9113();
  return;
}
