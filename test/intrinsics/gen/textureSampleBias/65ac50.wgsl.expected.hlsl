Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleBias_65ac50() {
  float4 res = arg_0.SampleBias(arg_1, float3(0.0f, 0.0f, float(1)), 1.0f, int2(0, 0));
}

void fragment_main() {
  textureSampleBias_65ac50();
  return;
}
