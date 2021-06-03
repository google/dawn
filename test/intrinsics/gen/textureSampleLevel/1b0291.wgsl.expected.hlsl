TextureCube arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_1b0291() {
  float res = arg_0.SampleLevel(arg_1, float3(0.0f, 0.0f, 0.0f), 1);
}

void vertex_main() {
  textureSampleLevel_1b0291();
  return;
}

void fragment_main() {
  textureSampleLevel_1b0291();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_1b0291();
  return;
}

