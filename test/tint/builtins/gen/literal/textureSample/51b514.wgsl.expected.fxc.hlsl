Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_51b514() {
  float4 res = arg_0.Sample(arg_1, (1.0f).xx);
}

void fragment_main() {
  textureSample_51b514();
  return;
}
