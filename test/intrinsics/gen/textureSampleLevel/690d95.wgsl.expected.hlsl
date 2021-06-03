Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_690d95() {
  float4 res = arg_0.SampleLevel(arg_1, float2(0.0f, 0.0f), 1.0f, int2(0, 0));
}

void vertex_main() {
  textureSampleLevel_690d95();
  return;
}

void fragment_main() {
  textureSampleLevel_690d95();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_690d95();
  return;
}

