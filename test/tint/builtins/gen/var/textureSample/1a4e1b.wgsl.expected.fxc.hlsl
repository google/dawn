Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_1a4e1b() {
  float2 arg_2 = (0.0f).xx;
  uint arg_3 = 1u;
  float res = arg_0.Sample(arg_1, float3(arg_2, float(arg_3))).x;
}

void fragment_main() {
  textureSample_1a4e1b();
  return;
}
