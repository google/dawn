TextureCubeArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSample_c2f4e8() {
  float3 arg_2 = (1.0f).xxx;
  int arg_3 = 1;
  float res = arg_0.Sample(arg_1, float4(arg_2, float(arg_3))).x;
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  textureSample_c2f4e8();
  return;
}
