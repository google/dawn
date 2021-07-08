Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_47daa4() {
  float res = arg_0.SampleLevel(arg_1, float2(0.0f, 0.0f), 0, int2(0, 0)).x;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureSampleLevel_47daa4();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureSampleLevel_47daa4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_47daa4();
  return;
}
