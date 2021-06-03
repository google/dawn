SKIP: FAILED



Validation Failure:
TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

void textureSampleCompare_a3ca7e() {
  float res = arg_0.SampleCmpLevelZero(arg_1, float4(float(1)), 1.0f);
}

void fragment_main() {
  textureSampleCompare_a3ca7e();
  return;
}


tint_IPHMMc:5:54: error: too few elements in vector initialization (expected 4 elements, have 1)
  float res = arg_0.SampleCmpLevelZero(arg_1, float4(float(1)), 1.0f);
                                                     ^

