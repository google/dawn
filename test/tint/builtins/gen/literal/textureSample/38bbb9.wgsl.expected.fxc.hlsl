Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_38bbb9() {
  float res = arg_0.Sample(arg_1, (1.0f).xx).x;
}

void fragment_main() {
  textureSample_38bbb9();
  return;
}
