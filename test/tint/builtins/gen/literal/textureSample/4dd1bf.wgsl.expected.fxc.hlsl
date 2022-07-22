TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_4dd1bf() {
  float4 res = arg_0.Sample(arg_1, float4(0.0f, 0.0f, 0.0f, float(1)));
}

void fragment_main() {
  textureSample_4dd1bf();
  return;
}
