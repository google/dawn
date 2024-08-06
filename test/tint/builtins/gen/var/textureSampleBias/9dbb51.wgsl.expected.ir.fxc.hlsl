
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleBias_9dbb51() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = 1;
  float arg_4 = 1.0f;
  Texture2DArray<float4> v = arg_0;
  SamplerState v_1 = arg_1;
  float2 v_2 = arg_2;
  float v_3 = arg_4;
  float4 res = v.SampleBias(v_1, float3(v_2, float(arg_3)), v_3, (1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBias_9dbb51()));
}

