SKIP: FAILED



Validation Failure:
TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_0bdd9a() {
  float4 res = arg_0.SampleLevel(arg_1, float4(float(1)), 1.0f);
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


tint_QjIcR9:5:48: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.SampleLevel(arg_1, float4(float(1)), 1.0f);
                                               ^


tint_QjIcR9:5:48: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.SampleLevel(arg_1, float4(float(1)), 1.0f);
                                               ^


tint_QjIcR9:5:48: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.SampleLevel(arg_1, float4(float(1)), 1.0f);
                                               ^

