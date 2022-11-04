TextureCube<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_e53267() {
  float4 res = arg_0.Sample(arg_1, (1.0f).xxx);
}

void fragment_main() {
  textureSample_e53267();
  return;
}
