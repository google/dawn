Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_02be59() {
  float res = arg_0.SampleLevel(arg_1, float2(0.0f, 0.0f), 1);
}

void vertex_main() {
  textureSampleLevel_02be59();
  return;
}

void fragment_main() {
  textureSampleLevel_02be59();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_02be59();
  return;
}

