Texture2D arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

void textureSampleCompareLevel_f8121c() {
  float res = arg_0.SampleCmpLevelZero(arg_1, float2(0.0f, 0.0f), 1.0f, int2(0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureSampleCompareLevel_f8121c();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureSampleCompareLevel_f8121c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleCompareLevel_f8121c();
  return;
}
