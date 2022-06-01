Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_667d76() {
  float res = arg_0.Sample(arg_1, (0.0f).xx, (0).xx).x;
}

void fragment_main() {
  textureSample_667d76();
  return;
}
