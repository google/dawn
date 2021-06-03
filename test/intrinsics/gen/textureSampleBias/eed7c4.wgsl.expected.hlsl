SKIP: FAILED



Validation Failure:
TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleBias_eed7c4() {
  float4 res = arg_0.SampleBias(arg_1, float4(float(1)), 1.0f);
}

void fragment_main() {
  textureSampleBias_eed7c4();
  return;
}


tint_NFwEG1:5:47: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.SampleBias(arg_1, float4(float(1)), 1.0f);
                                              ^

