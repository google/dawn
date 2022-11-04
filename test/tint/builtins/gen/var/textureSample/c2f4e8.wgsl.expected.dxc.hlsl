TextureCubeArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_c2f4e8() {
  float3 arg_2 = (1.0f).xxx;
  int arg_3 = 1;
  float res = arg_0.Sample(arg_1, float4(arg_2, float(arg_3))).x;
}

void fragment_main() {
  textureSample_c2f4e8();
  return;
}
