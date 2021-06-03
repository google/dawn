SKIP: FAILED



Validation Failure:
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_a4af26() {
  float4 res = arg_0.SampleLevel(arg_1, float3(float(1)), 1.0f, int2(0, 0));
}

void vertex_main() {
  textureSampleLevel_a4af26();
  return;
}

void fragment_main() {
  textureSampleLevel_a4af26();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_a4af26();
  return;
}


tint_TRKrS9:5:48: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleLevel(arg_1, float3(float(1)), 1.0f, int2(0, 0));
                                               ^


tint_TRKrS9:5:48: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleLevel(arg_1, float3(float(1)), 1.0f, int2(0, 0));
                                               ^


tint_TRKrS9:5:48: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleLevel(arg_1, float3(float(1)), 1.0f, int2(0, 0));
                                               ^

