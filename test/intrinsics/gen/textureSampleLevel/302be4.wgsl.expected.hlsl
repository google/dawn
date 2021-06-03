SKIP: FAILED



Validation Failure:
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_302be4() {
  float4 res = arg_0.SampleLevel(arg_1, float3(float(1)), 1.0f);
}

void vertex_main() {
  textureSampleLevel_302be4();
  return;
}

void fragment_main() {
  textureSampleLevel_302be4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_302be4();
  return;
}


tint_wKzugP:5:48: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleLevel(arg_1, float3(float(1)), 1.0f);
                                               ^


tint_wKzugP:5:48: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleLevel(arg_1, float3(float(1)), 1.0f);
                                               ^


tint_wKzugP:5:48: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleLevel(arg_1, float3(float(1)), 1.0f);
                                               ^

