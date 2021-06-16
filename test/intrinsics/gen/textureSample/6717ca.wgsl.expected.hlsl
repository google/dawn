Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_6717ca() {
  float4 res = arg_0.Sample(arg_1, float3(0.0f, 0.0f, float(1)));
}

void fragment_main() {
  textureSample_6717ca();
  return;
}
