
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleBias_80e579() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = int(1);
  float arg_4 = 1.0f;
  float2 v = arg_2;
  float v_1 = clamp(arg_4, -16.0f, 15.9899997711181640625f);
  float4 res = arg_0.SampleBias(arg_1, float3(v, float(arg_3)), v_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBias_80e579()));
}

