TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_0bdd9a() {
  float4 res = arg_0.SampleLevel(arg_1, float4(0.0f, 0.0f, 0.0f, float(1)), 1.0f);
}

void vertex_main() {
  textureSampleLevel_0bdd9a();
  return;
}

void fragment_main() {
  textureSampleLevel_0bdd9a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_0bdd9a();
  return;
}

