SKIP: FAILED



Validation Failure:
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleGrad_872f00() {
  float4 res = arg_0.SampleGrad(arg_1, float3(float(1)), float2(0.0f, 0.0f), float2(0.0f, 0.0f), int2(0, 0));
}

void vertex_main() {
  textureSampleGrad_872f00();
  return;
}

void fragment_main() {
  textureSampleGrad_872f00();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleGrad_872f00();
  return;
}


tint_TQAGnF:5:47: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleGrad(arg_1, float3(float(1)), float2(0.0f, 0.0f), float2(0.0f, 0.0f), int2(0, 0));
                                              ^


tint_TQAGnF:5:47: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleGrad(arg_1, float3(float(1)), float2(0.0f, 0.0f), float2(0.0f, 0.0f), int2(0, 0));
                                              ^


tint_TQAGnF:5:47: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleGrad(arg_1, float3(float(1)), float2(0.0f, 0.0f), float2(0.0f, 0.0f), int2(0, 0));
                                              ^

