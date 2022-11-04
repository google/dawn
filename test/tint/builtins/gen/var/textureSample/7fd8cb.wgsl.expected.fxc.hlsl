TextureCubeArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_7fd8cb() {
  float3 arg_2 = (1.0f).xxx;
  uint arg_3 = 1u;
  float res = arg_0.Sample(arg_1, float4(arg_2, float(arg_3))).x;
}

void fragment_main() {
  textureSample_7fd8cb();
  return;
}
