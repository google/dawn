
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleBias_87915c() {
  float4 res = arg_0.SampleBias(arg_1, float3((1.0f).xx, float(1u)), clamp(1.0f, -16.0f, 15.9899997711181640625f), (int(1)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBias_87915c()));
}

