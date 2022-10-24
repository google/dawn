Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_4703d0() {
  float res = arg_0.Sample(arg_1, float3(0.0f, 0.0f, float(1u)), (0).xx).x;
}

void fragment_main() {
  textureSample_4703d0();
  return;
}
