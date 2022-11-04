Texture3D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_3b50bd() {
  float3 arg_2 = (1.0f).xxx;
  float4 res = arg_0.Sample(arg_1, arg_2);
}

void fragment_main() {
  textureSample_3b50bd();
  return;
}
