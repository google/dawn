Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleBias_9dbb51() {
  float4 res = arg_0.SampleBias(arg_1, float3((1.0f).xx, float(1)), 1.0f, int2((1).xx));
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  textureSampleBias_9dbb51();
  return;
}
