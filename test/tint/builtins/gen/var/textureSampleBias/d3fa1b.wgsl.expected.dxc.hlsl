Texture3D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleBias_d3fa1b() {
  float3 arg_2 = (1.0f).xxx;
  float arg_3 = 1.0f;
  float4 res = arg_0.SampleBias(arg_1, arg_2, arg_3);
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  textureSampleBias_d3fa1b();
  return;
}
