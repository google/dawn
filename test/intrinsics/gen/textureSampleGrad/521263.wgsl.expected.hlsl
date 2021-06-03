Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleGrad_521263() {
  float4 res = arg_0.SampleGrad(arg_1, float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  textureSampleGrad_521263();
  return;
}

void fragment_main() {
  textureSampleGrad_521263();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleGrad_521263();
  return;
}

