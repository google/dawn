Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_193203() {
  float2 arg_2 = (0.0f).xx;
  uint arg_3 = 1u;
  float4 res = arg_0.Sample(arg_1, float3(arg_2, float(arg_3)), (0).xx);
}

void fragment_main() {
  textureSample_193203();
  return;
}
