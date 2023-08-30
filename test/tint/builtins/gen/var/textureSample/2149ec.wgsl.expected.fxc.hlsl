Texture3D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSample_2149ec() {
  float3 arg_2 = (1.0f).xxx;
  float4 res = arg_0.Sample(arg_1, arg_2, int3((1).xxx));
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  textureSample_2149ec();
  return;
}
