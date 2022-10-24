TextureCubeArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_7fd8cb() {
  float res = arg_0.Sample(arg_1, float4(0.0f, 0.0f, 0.0f, float(1u))).x;
}

void fragment_main() {
  textureSample_7fd8cb();
  return;
}
