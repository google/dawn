Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_667d76() {
  float res = arg_0.Sample(arg_1, float2(0.0f, 0.0f), int2(0, 0)).x;
}

void fragment_main() {
  textureSample_667d76();
  return;
}
