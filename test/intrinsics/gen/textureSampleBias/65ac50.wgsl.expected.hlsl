SKIP: FAILED



Validation Failure:
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleBias_65ac50() {
  float4 res = arg_0.SampleBias(arg_1, float3(float(1)), 1.0f, int2(0, 0));
}

void fragment_main() {
  textureSampleBias_65ac50();
  return;
}


tint_faUSzc:5:47: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.SampleBias(arg_1, float3(float(1)), 1.0f, int2(0, 0));
                                              ^

