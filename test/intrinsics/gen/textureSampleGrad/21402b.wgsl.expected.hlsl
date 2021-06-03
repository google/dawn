Texture3D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleGrad_21402b() {
  float4 res = arg_0.SampleGrad(arg_1, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  textureSampleGrad_21402b();
  return;
}

void fragment_main() {
  textureSampleGrad_21402b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleGrad_21402b();
  return;
}

