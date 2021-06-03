Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_ba93b3() {
  float res = arg_0.SampleLevel(arg_1, float3(0.0f, 0.0f, float(1)), 1, int2(0, 0));
}

void vertex_main() {
  textureSampleLevel_ba93b3();
  return;
}

void fragment_main() {
  textureSampleLevel_ba93b3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_ba93b3();
  return;
}

