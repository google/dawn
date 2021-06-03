SKIP: FAILED



Validation Failure:
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleGrad_2ecd8f() {
  float4 res = arg_0.SampleGrad(arg_1, float3(float(1)), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  textureSampleGrad_2ecd8f();
  return;
}

void fragment_main() {
  textureSampleGrad_2ecd8f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleGrad_2ecd8f();
  return;
}


tint_Qg4FBC:5:47: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleGrad(arg_1, float3(float(1)), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
                                              ^


tint_Qg4FBC:5:47: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleGrad(arg_1, float3(float(1)), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
                                              ^


tint_Qg4FBC:5:47: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleGrad(arg_1, float3(float(1)), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
                                              ^

