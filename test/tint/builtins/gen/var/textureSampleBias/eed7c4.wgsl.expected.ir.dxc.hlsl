
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleBias_eed7c4() {
  float3 arg_2 = (1.0f).xxx;
  int arg_3 = int(1);
  float arg_4 = 1.0f;
  TextureCubeArray<float4> v = arg_0;
  SamplerState v_1 = arg_1;
  float3 v_2 = arg_2;
  float v_3 = arg_4;
  float4 res = v.SampleBias(v_1, float4(v_2, float(arg_3)), v_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBias_eed7c4()));
}

