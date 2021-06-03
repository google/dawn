SKIP: FAILED



Validation Failure:
Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

void textureSampleCompare_98b85c() {
  float res = arg_0.SampleCmpLevelZero(arg_1, float3(float(1)), 1.0f, int2(0, 0));
}

void fragment_main() {
  textureSampleCompare_98b85c();
  return;
}


tint_A6uvj1:5:54: error: too few elements in vector initialization (expected 3 elements, have 1)
  float res = arg_0.SampleCmpLevelZero(arg_1, float3(float(1)), 1.0f, int2(0, 0));
                                                     ^

